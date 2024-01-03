/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#include <base/platform.h>

#include "graphics_p.h"
#include <graphics/embedded_shader.h>
#include <base/file.h>
#include <base/mutex.h>

#include "topology.h"

#if BASE_PLATFORM_OSX || BASE_PLATFORM_IOS
#	include <objc/message.h>
#elif BASE_PLATFORM_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif // WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif // BASE_PLATFORM_OSX

BASE_ERROR_RESULT(GRAPHICS_ERROR_TEXTURE_VALIDATION,      BASE_MAKEFOURCC('b', 'g', 0, 1) );
BASE_ERROR_RESULT(GRAPHICS_ERROR_FRAME_BUFFER_VALIDATION, BASE_MAKEFOURCC('b', 'g', 0, 2) );
BASE_ERROR_RESULT(GRAPHICS_ERROR_IDENTIFIER_VALIDATION,   BASE_MAKEFOURCC('b', 'g', 0, 3) );

namespace graphics
{
#define GRAPHICS_API_THREAD_MAGIC UINT32_C(0x78666762)

#if GRAPHICS_CONFIG_MULTITHREADED

#	define GRAPHICS_CHECK_API_THREAD()                                   \
		BASE_ASSERT(NULL != s_ctx, "Library is not initialized yet."); \
		BASE_ASSERT(GRAPHICS_API_THREAD_MAGIC == s_threadIndex, "Must be called from main thread.")

#	define GRAPHICS_CHECK_RENDER_THREAD()                         \
		BASE_ASSERT( (NULL != s_ctx && s_ctx->m_singleThreaded) \
			|| ~GRAPHICS_API_THREAD_MAGIC == s_threadIndex        \
			, "Must be called from render thread."            \
			)

#else
#	define GRAPHICS_CHECK_API_THREAD()
#	define GRAPHICS_CHECK_RENDER_THREAD()
#endif // GRAPHICS_CONFIG_MULTITHREADED

#define GRAPHICS_CHECK_CAPS(_caps, _msg)                                                   \
	BASE_ASSERT(0 != (g_caps.supported & (_caps) )                                       \
		, _msg " Use graphics::getCaps to check " #_caps " backend renderer capabilities." \
		);

#if GRAPHICS_CONFIG_USE_TINYSTL
	void* TinyStlAllocator::static_allocate(size_t _bytes)
	{
		return base::alloc(g_allocator, _bytes);
	}

	void TinyStlAllocator::static_deallocate(void* _ptr, size_t /*_bytes*/)
	{
		if (NULL != _ptr)
		{
			base::free(g_allocator, _ptr);
		}
	}
#endif // GRAPHICS_CONFIG_USE_TINYSTL

	struct CallbackStub : public CallbackI
	{
		virtual ~CallbackStub()
		{
		}

		virtual void fatal(const char* _filePath, uint16_t _line, Fatal::Enum _code, const char* _str) override
		{
			graphics::trace(_filePath, _line, "GRAPHICS FATAL 0x%08x: %s\n", _code, _str);

			if (Fatal::DebugCheck == _code)
			{
				base::debugBreak();
			}
			else
			{
				abort();
			}
		}

		virtual void traceVargs(const char* _filePath, uint16_t _line, const char* _format, va_list _argList) override
		{
			char temp[2048];
			char* out = temp;
			va_list argListCopy;
			va_copy(argListCopy, _argList);
			int32_t len   = base::snprintf(out, sizeof(temp), "%s (%d): ", _filePath, _line);
			int32_t total = len + base::vsnprintf(out + len, sizeof(temp)-len, _format, argListCopy);
			va_end(argListCopy);
			if ( (int32_t)sizeof(temp) < total)
			{
				out = (char*)alloca(total+1);
				base::memCopy(out, temp, len);
				base::vsnprintf(out + len, total-len, _format, _argList);
			}
			out[total] = '\0';
			base::debugOutput(out);
		}

		virtual void profilerBegin(const char* /*_name*/, uint32_t /*_abgr*/, const char* /*_filePath*/, uint16_t /*_line*/) override
		{
		}

		virtual void profilerBeginLiteral(const char* /*_name*/, uint32_t /*_abgr*/, const char* /*_filePath*/, uint16_t /*_line*/) override
		{
		}

		virtual void profilerEnd() override
		{
		}

		virtual uint32_t cacheReadSize(uint64_t /*_id*/) override
		{
			return 0;
		}

		virtual bool cacheRead(uint64_t /*_id*/, void* /*_data*/, uint32_t /*_size*/) override
		{
			return false;
		}

		virtual void cacheWrite(uint64_t /*_id*/, const void* /*_data*/, uint32_t /*_size*/) override
		{
		}

		virtual void screenShot(const char* _filePath, uint32_t _width, uint32_t _height, uint32_t _pitch, const void* _data, uint32_t _size, bool _yflip) override
		{
			BASE_UNUSED(_filePath, _width, _height, _pitch, _data, _size, _yflip);

			const int32_t len = base::strLen(_filePath)+5;
			char* filePath = (char*)alloca(len);
			base::strCopy(filePath, len, _filePath);
			base::strCat(filePath, len, ".tga");

			base::FileWriter writer;
			if (base::open(&writer, filePath) )
			{
				bimg::imageWriteTga(&writer, _width, _height, _pitch, _data, false, _yflip);
				base::close(&writer);
			}
		}

		virtual void captureBegin(uint32_t /*_width*/, uint32_t /*_height*/, uint32_t /*_pitch*/, TextureFormat::Enum /*_format*/, bool /*_yflip*/) override
		{
			BASE_TRACE("Warning: using capture without callback (a.k.a. pointless).");
		}

		virtual void captureEnd() override
		{
		}

		virtual void captureFrame(const void* /*_data*/, uint32_t /*_size*/) override
		{
		}
	};

#ifndef GRAPHICS_CONFIG_MEMORY_TRACKING
#	define GRAPHICS_CONFIG_MEMORY_TRACKING (GRAPHICS_CONFIG_DEBUG && BASE_CONFIG_SUPPORTS_THREADING)
#endif // GRAPHICS_CONFIG_MEMORY_TRACKING

	const size_t kNaturalAlignment = 8;

	class AllocatorStub : public base::AllocatorI
	{
	public:
		AllocatorStub()
#if GRAPHICS_CONFIG_MEMORY_TRACKING
			: m_numBlocks(0)
			, m_maxBlocks(0)
#endif // GRAPHICS_CONFIG_MEMORY_TRACKING
		{
		}

		virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line) override
		{
			if (0 == _size)
			{
				if (NULL != _ptr)
				{
					if (kNaturalAlignment >= _align)
					{
#if GRAPHICS_CONFIG_MEMORY_TRACKING
						{
							base::MutexScope scope(m_mutex);
							BASE_ASSERT(m_numBlocks > 0, "Number of blocks is 0. Possible alloc/free mismatch?");
							--m_numBlocks;
						}
#endif // GRAPHICS_CONFIG_MEMORY_TRACKING

						::free(_ptr);
					}
					else
					{
						base::alignedFree(this, _ptr, _align, base::Location(_file, _line) );
					}
				}

				return NULL;
			}
			else if (NULL == _ptr)
			{
				if (kNaturalAlignment >= _align)
				{
#if GRAPHICS_CONFIG_MEMORY_TRACKING
					{
						base::MutexScope scope(m_mutex);
						++m_numBlocks;
						m_maxBlocks = base::max(m_maxBlocks, m_numBlocks);
					}
#endif // GRAPHICS_CONFIG_MEMORY_TRACKING

					return ::malloc(_size);
				}

				return base::alignedAlloc(this, _size, _align, base::Location(_file, _line) );
			}

			if (kNaturalAlignment >= _align)
			{
#if GRAPHICS_CONFIG_MEMORY_TRACKING
				if (NULL == _ptr)
				{
					base::MutexScope scope(m_mutex);
					++m_numBlocks;
					m_maxBlocks = base::max(m_maxBlocks, m_numBlocks);
				}
#endif // GRAPHICS_CONFIG_MEMORY_TRACKING

				return ::realloc(_ptr, _size);
			}

			return base::alignedRealloc(this, _ptr, _size, _align, base::Location(_file, _line) );
		}

		void checkLeaks();

	protected:
#if GRAPHICS_CONFIG_MEMORY_TRACKING
		base::Mutex m_mutex;
		uint32_t m_numBlocks;
		uint32_t m_maxBlocks;
#endif // GRAPHICS_CONFIG_MEMORY_TRACKING
	};

	static CallbackStub*  s_callbackStub  = NULL;
	static AllocatorStub* s_allocatorStub = NULL;
	static bool s_graphicsDebuggerPresent = false;

	CallbackI* g_callback = NULL;
	base::AllocatorI* g_allocator = NULL;

	Caps g_caps;

#if GRAPHICS_CONFIG_MULTITHREADED && !defined(BASE_THREAD_LOCAL)
	class ThreadData
	{
		BASE_CLASS(ThreadData
			, NO_DEFAULT_CTOR
			, NO_COPY
			);

	public:
		ThreadData(uintptr_t _rhs)
		{
			union { uintptr_t ui; void* ptr; } cast = { _rhs };
			m_tls.set(cast.ptr);
		}

		operator uintptr_t() const
		{
			union { uintptr_t ui; void* ptr; } cast;
			cast.ptr = m_tls.get();
			return cast.ui;
		}

		uintptr_t operator=(uintptr_t _rhs)
		{
			union { uintptr_t ui; void* ptr; } cast = { _rhs };
			m_tls.set(cast.ptr);
			return _rhs;
		}

		bool operator==(uintptr_t _rhs) const
		{
			uintptr_t lhs = *this;
			return lhs == _rhs;
		}

	private:
		base::TlsData m_tls;
	};

	static ThreadData s_threadIndex(0);
#elif !GRAPHICS_CONFIG_MULTITHREADED
	static uint32_t s_threadIndex(0);
#else
	static BASE_THREAD_LOCAL uint32_t s_threadIndex(0);
#endif

	static Context* s_ctx = NULL;
	static bool s_renderFrameCalled = false;
	InternalData g_internalData;
	PlatformData g_platformData;
	bool g_platformDataChangedSinceReset = false;

	static Handle::TypeName s_typeName[] =
	{
		{ "DIB",  "DynamicIndexBuffer"  },
		{ "DVB",  "DynamicVertexBuffer" },
		{ "FB",   "FrameBuffer"         },
		{ "IB",   "IndexBuffer"         },
		{ "IndB", "IndirectBuffer"      },
		{ "OQ",   "OcclusionQuery"      },
		{ "P",    "Program"             },
		{ "S",    "Shader"              },
		{ "T",    "Texture"             },
		{ "U",    "Uniform"             },
		{ "VB",   "VertexBuffer"        },
		{ "VL",   "VertexLayout"        },
		{ "?",    "?"                   },
	};
	BASE_STATIC_ASSERT(BASE_COUNTOF(s_typeName) == Handle::Count+1, "");

	const Handle::TypeName& Handle::getTypeName(Handle::Enum _enum)
	{
		BASE_ASSERT(_enum < Handle::Count, "Invalid Handle::Enum %d!", _enum);
		return s_typeName[base::min(_enum, Handle::Count)];
	}

	void AllocatorStub::checkLeaks()
	{
#if GRAPHICS_CONFIG_MEMORY_TRACKING
		// BK - CallbackStub will be deleted after printing this info, so there is always one
		// leak if CallbackStub is used.
		BASE_WARN(uint32_t(NULL != s_callbackStub ? 1 : 0) == m_numBlocks
			, "\n\n"
			  "\n########################################################"
			  "\n"
			  "\nMEMORY LEAK: Number of leaked blocks %d (Max blocks: %d)"
			  "\n"
			  "\n########################################################"
			  "\n\n"
			, m_numBlocks
			, m_maxBlocks
			);
#endif // GRAPHICS_CONFIG_MEMORY_TRACKING
	}

	void setPlatformData(const PlatformData& _data)
	{
		if (NULL != s_ctx)
		{
			GRAPHICS_FATAL(true
				&& g_platformData.ndt     == _data.ndt
				&& g_platformData.context == _data.context
				, Fatal::UnableToInitialize
				, "Only backbuffer pointer and native window handle can be changed after initialization!"
				);
		}
		base::memCopy(&g_platformData, &_data, sizeof(PlatformData) );
		g_platformDataChangedSinceReset = true;
	}

	const InternalData* getInternalData()
	{
		return &g_internalData;
	}

	uintptr_t overrideInternal(TextureHandle _handle, uintptr_t _ptr)
	{
		GRAPHICS_CHECK_RENDER_THREAD();
		RendererContextI* rci = s_ctx->m_renderCtx;
		if (0 == rci->getInternal(_handle) )
		{
			return 0;
		}

		rci->overrideInternal(_handle, _ptr);

		return rci->getInternal(_handle);
	}

	uintptr_t overrideInternal(TextureHandle _handle, uint16_t _width, uint16_t _height, uint8_t _numMips, TextureFormat::Enum _format, uint64_t _flags)
	{
		GRAPHICS_CHECK_RENDER_THREAD();
		RendererContextI* rci = s_ctx->m_renderCtx;
		if (0 == rci->getInternal(_handle) )
		{
			return 0;
		}

		uint32_t size = sizeof(uint32_t) + sizeof(TextureCreate);
		Memory* mem = const_cast<Memory*>(alloc(size) );

		base::StaticMemoryBlockWriter writer(mem->data, mem->size);
		uint32_t magic = GRAPHICS_CHUNK_MAGIC_TEX;
		base::write(&writer, magic, base::ErrorAssert{});

		TextureCreate tc;
		tc.m_width     = _width;
		tc.m_height    = _height;
		tc.m_depth     = 0;
		tc.m_numLayers = 1;
		tc.m_numMips   = base::max<uint8_t>(1, _numMips);
		tc.m_format    = _format;
		tc.m_cubeMap   = false;
		tc.m_mem       = NULL;
		base::write(&writer, tc, base::ErrorAssert{});

		rci->destroyTexture(_handle);
		rci->createTexture(_handle, mem, _flags, 0);

		release(mem);

		return rci->getInternal(_handle);
	}

	void setGraphicsDebuggerPresent(bool _present)
	{
		BASE_TRACE("Graphics debugger is %spresent.", _present ? "" : "not ");
		s_graphicsDebuggerPresent = _present;
	}

	bool isGraphicsDebuggerPresent()
	{
		return s_graphicsDebuggerPresent;
	}

	void fatal(const char* _filePath, uint16_t _line, Fatal::Enum _code, const char* _format, ...)
	{
		va_list argList;
		va_start(argList, _format);

		char temp[8192];
		char* out = temp;
		int32_t len = base::vsnprintf(out, sizeof(temp), _format, argList);
		if ( (int32_t)sizeof(temp) < len)
		{
			out = (char*)alloca(len+1);
			len = base::vsnprintf(out, len, _format, argList);
		}
		out[len] = '\0';

		if (BASE_UNLIKELY(NULL == g_callback) )
		{
			base::debugPrintf("%s(%d): GRAPHICS FATAL 0x%08x: %s", _filePath, _line, _code, out);
			abort();
		}
		else
		{
			g_callback->fatal(_filePath, _line, _code, out);
		}

		va_end(argList);
	}

	void trace(const char* _filePath, uint16_t _line, const char* _format, ...)
	{
		va_list argList;
		va_start(argList, _format);

		if (BASE_UNLIKELY(NULL == g_callback) )
		{
			base::debugPrintfVargs(_format, argList);
		}
		else
		{
			g_callback->traceVargs(_filePath, _line, _format, argList);
		}

		va_end(argList);
	}

#include "vs_debugfont.bin.h"
#include "fs_debugfont.bin.h"
#include "vs_clear.bin.h"
#include "fs_clear0.bin.h"
#include "fs_clear1.bin.h"
#include "fs_clear2.bin.h"
#include "fs_clear3.bin.h"
#include "fs_clear4.bin.h"
#include "fs_clear5.bin.h"
#include "fs_clear6.bin.h"
#include "fs_clear7.bin.h"

	static const EmbeddedShader s_embeddedShaders[] =
	{
		GRAPHICS_EMBEDDED_SHADER(vs_debugfont),
		GRAPHICS_EMBEDDED_SHADER(fs_debugfont),
		GRAPHICS_EMBEDDED_SHADER(vs_clear),
		GRAPHICS_EMBEDDED_SHADER(fs_clear0),
		GRAPHICS_EMBEDDED_SHADER(fs_clear1),
		GRAPHICS_EMBEDDED_SHADER(fs_clear2),
		GRAPHICS_EMBEDDED_SHADER(fs_clear3),
		GRAPHICS_EMBEDDED_SHADER(fs_clear4),
		GRAPHICS_EMBEDDED_SHADER(fs_clear5),
		GRAPHICS_EMBEDDED_SHADER(fs_clear6),
		GRAPHICS_EMBEDDED_SHADER(fs_clear7),

		GRAPHICS_EMBEDDED_SHADER_END()
	};

	ShaderHandle createEmbeddedShader(const EmbeddedShader* _es, RendererType::Enum _type, const char* _name)
	{
		for (const EmbeddedShader* es = _es; NULL != es->name; ++es)
		{
			if (0 == base::strCmp(_name, es->name) )
			{
				for (const EmbeddedShader::Data* esd = es->data; RendererType::Count != esd->type; ++esd)
				{
					if (_type == esd->type
					&&  1 < esd->size)
					{
						ShaderHandle handle = createShader(makeRef(esd->data, esd->size) );
						if (isValid(handle) )
						{
							setName(handle, _name);
						}

						return handle;
					}
				}
			}
		}

		ShaderHandle handle = GRAPHICS_INVALID_HANDLE;
		return handle;
	}

	void dump(const VertexLayout& _layout)
	{
		if (BASE_ENABLED(GRAPHICS_CONFIG_DEBUG) )
		{
			BASE_TRACE("VertexLayout %08x (%08x), stride %d"
				, _layout.m_hash
				, base::hash<base::HashMurmur2A>(_layout.m_attributes)
				, _layout.m_stride
				);

			for (uint32_t attr = 0; attr < Attrib::Count; ++attr)
			{
				if (UINT16_MAX != _layout.m_attributes[attr])
				{
					uint8_t num;
					AttribType::Enum type;
					bool normalized;
					bool asInt;
					_layout.decode(Attrib::Enum(attr), num, type, normalized, asInt);

					BASE_TRACE("\tattr %2d: %-20s num %d, type %d, norm [%c], asint [%c], offset %2d"
						, attr
						, getAttribName(Attrib::Enum(attr) )
						, num
						, type
						, normalized ? 'x' : ' '
						, asInt      ? 'x' : ' '
						, _layout.m_offset[attr]
						);
				}
			}
		}
	}

#include "charset.h"

	void charsetFillTexture(const uint8_t* _charset, uint8_t* _rgba, uint32_t _height, uint32_t _pitch, uint32_t _bpp)
	{
		for (uint32_t ii = 0; ii < 256; ++ii)
		{
			uint8_t* pix = &_rgba[ii*8*_bpp];
			for (uint32_t yy = 0; yy < _height; ++yy)
			{
				for (uint32_t xx = 0; xx < 8; ++xx)
				{
					uint8_t bit = 1<<(7-xx);
					base::memSet(&pix[xx*_bpp], _charset[ii*_height+yy]&bit ? 255 : 0, _bpp);
				}

				pix += _pitch;
			}
		}
	}

	static uint8_t parseAttrTo(char*& _ptr, char _to, uint8_t _default)
	{
		const base::StringView str = base::strFind(_ptr, _to);
		if (!str.isEmpty()
		&&  3 > str.getPtr()-_ptr)
		{
			char tmp[4];

			int32_t len = int32_t(str.getPtr()-_ptr);
			base::strCopy(tmp, sizeof(tmp), _ptr, len);

			uint32_t attr;
			base::fromString(&attr, tmp);

			_ptr += len+1;
			return uint8_t(attr);
		}

		return _default;
	}

	static uint8_t parseAttr(char*& _ptr, uint8_t _default)
	{
		char* ptr = _ptr;
		if (*ptr++ != '[')
		{
			return _default;
		}

		if (0 == base::strCmp(ptr, "0m", 2) )
		{
			_ptr = ptr + 2;
			return _default;
		}

		uint8_t fg = parseAttrTo(ptr, ';', _default & 0xf);
		uint8_t bg = parseAttrTo(ptr, 'm', _default >> 4);

		uint8_t attr = (bg<<4) | fg;
		_ptr = ptr;
		return attr;
	}

	void TextVideoMem::printfVargs(uint16_t _x, uint16_t _y, uint8_t _attr, const char* _format, va_list _argList)
	{
		if (_x < m_width && _y < m_height)
		{
			va_list argListCopy;
			va_copy(argListCopy, _argList);
			uint32_t num = base::vsnprintf(NULL, 0, _format, argListCopy) + 1;
			char* temp = (char*)alloca(num);
			va_copy(argListCopy, _argList);
			num = base::vsnprintf(temp, num, _format, argListCopy);

			uint8_t attr = _attr;
			MemSlot* mem = &m_mem[_y*m_width+_x];
			for (uint32_t ii = 0, xx = _x; ii < num && xx < m_width; ++ii)
			{
				char ch = temp[ii];
				if (BASE_UNLIKELY(ch == '\x1b') )
				{
					char* ptr = &temp[ii+1];
					attr = parseAttr(ptr, _attr);
					ii += uint32_t(ptr - &temp[ii+1]);
				}
				else
				{
					mem->character = ch;
					mem->attribute = attr;
					++mem;
					++xx;
				}
			}
		}
	}

	static const uint32_t numCharsPerBatch = 1024;
	static const uint32_t numBatchVertices = numCharsPerBatch*4;
	static const uint32_t numBatchIndices  = numCharsPerBatch*6;

	void TextVideoMemBlitter::init(uint8_t scale)
	{
		GRAPHICS_CHECK_API_THREAD();
		m_layout
			.begin()
			.add(Attrib::Position,  3, AttribType::Float)
			.add(Attrib::Color0,    4, AttribType::Uint8, true)
			.add(Attrib::Color1,    4, AttribType::Uint8, true)
			.add(Attrib::TexCoord0, 2, AttribType::Float)
			.end();

		uint16_t width  = 2048;
		uint16_t height = 24;
		uint8_t  bpp    = 1;
		uint32_t pitch  = width*bpp;

		const Memory* mem;

		mem = alloc(pitch*height);
		uint8_t* rgba = mem->data;
		charsetFillTexture(vga8x8, rgba, 8, pitch, bpp);
		charsetFillTexture(vga8x16, &rgba[8*pitch], 16, pitch, bpp);
		m_texture = createTexture2D(width, height, false, 1, TextureFormat::R8
			, GRAPHICS_SAMPLER_MIN_POINT
			| GRAPHICS_SAMPLER_MAG_POINT
			| GRAPHICS_SAMPLER_MIP_POINT
			| GRAPHICS_SAMPLER_U_CLAMP
			| GRAPHICS_SAMPLER_V_CLAMP
			, mem
			);

		ShaderHandle vsh = createEmbeddedShader(s_embeddedShaders, g_caps.rendererType, "vs_debugfont");
		ShaderHandle fsh = createEmbeddedShader(s_embeddedShaders, g_caps.rendererType, "fs_debugfont");

		BASE_ASSERT(isValid(vsh) && isValid(fsh), "Failed to create embedded blit shaders");

		m_program = createProgram(vsh, fsh, true);

		m_vb = s_ctx->createTransientVertexBuffer(numBatchVertices*m_layout.m_stride, &m_layout);
		m_ib = s_ctx->createTransientIndexBuffer(numBatchIndices*2);
		m_scale = base::max<uint8_t>(scale, 1);
	}

	void TextVideoMemBlitter::shutdown()
	{
		GRAPHICS_CHECK_API_THREAD();

		if (isValid(m_program) )
		{
			destroy(m_program);
		}

		destroy(m_texture);
		s_ctx->destroyTransientVertexBuffer(m_vb);
		s_ctx->destroyTransientIndexBuffer(m_ib);
	}

	static const uint32_t s_paletteSrgb[] =
	{
		0x0,        // Black
		0xffa46534, // Blue
		0xff069a4e, // Green
		0xff9a9806, // Cyan
		0xff0000cc, // Red
		0xff7b5075, // Magenta
		0xff00a0c4, // Brown
		0xffcfd7d3, // Light Gray
		0xff535755, // Dark Gray
		0xffcf9f72, // Light Blue
		0xff34e28a, // Light Green
		0xffe2e234, // Light Cyan
		0xff2929ef, // Light Red
		0xffa87fad, // Light Magenta
		0xff4fe9fc, // Yellow
		0xffeceeee, // White
	};
	BASE_STATIC_ASSERT(BASE_COUNTOF(s_paletteSrgb) == 16);

	static const uint32_t s_paletteLinear[] =
	{
		0x0,        // Black
		0xff5e2108, // Blue
		0xff005213, // Green
		0xff525000, // Cyan
		0xff000099, // Red
		0xff32142d, // Magenta
		0xff00598c, // Brown
		0xff9fada6, // Light Gray
		0xff161817, // Dark Gray
		0xff9f582a, // Light Blue
		0xff08c140, // Light Green
		0xffc1c108, // Light Cyan
		0xff0505dc, // Light Red
		0xff63366a, // Light Magenta
		0xff13cff8, // Yellow
		0xffd5dada  // White
	};
	BASE_STATIC_ASSERT(BASE_COUNTOF(s_paletteLinear) == 16);

	void blit(RendererContextI* _renderCtx, TextVideoMemBlitter& _blitter, const TextVideoMem& _mem)
	{
		struct Vertex
		{
			float m_x;
			float m_y;
			float m_z;
			uint32_t m_fg;
			uint32_t m_bg;
			float m_u;
			float m_v;
		};

		uint32_t yy = 0;
		uint32_t xx = 0;

		const float texelWidth      = 1.0f/2048.0f;
		const float texelWidthHalf  = RendererType::Direct3D9 == g_caps.rendererType ? 0.0f : texelWidth*0.5f;
		const float texelHeight     = 1.0f/24.0f;
		const float texelHeightHalf = RendererType::Direct3D9 == g_caps.rendererType ? texelHeight*0.5f : 0.0f;
		const float utop       = (_mem.m_small ? 0.0f :  8.0f)*texelHeight + texelHeightHalf;
		const float ubottom    = (_mem.m_small ? 8.0f : 24.0f)*texelHeight + texelHeightHalf;
		const float fontHeight = (_mem.m_small ? 8.0f : 16.0f)*_blitter.m_scale;
		const float fontWidth  = 8.0f * _blitter.m_scale;

		_renderCtx->blitSetup(_blitter);

		const uint32_t* palette = 0 != (s_ctx->m_init.resolution.reset & GRAPHICS_RESET_SRGB_BACKBUFFER)
			? s_paletteLinear
			: s_paletteSrgb
			;

		for (;yy < _mem.m_height;)
		{
			Vertex* vertex = (Vertex*)_blitter.m_vb->data;
			uint16_t* indices = (uint16_t*)_blitter.m_ib->data;
			uint32_t startVertex = 0;
			uint32_t numIndices = 0;

			for (; yy < _mem.m_height && numIndices < numBatchIndices; ++yy)
			{
				xx = xx < _mem.m_width ? xx : 0;
				const TextVideoMem::MemSlot* line = &_mem.m_mem[yy*_mem.m_width+xx];

				for (; xx < _mem.m_width && numIndices < numBatchIndices; ++xx)
				{
					uint32_t ch = line->character;
					const uint8_t attr = line->attribute;

					if (ch > 0xff)
					{
						ch = 0;
					}

					if (0 != (ch|attr)
					&& (' ' != ch || 0 != (attr&0xf0) ) )
					{
						const uint32_t fg = palette[attr&0xf];
						const uint32_t bg = palette[(attr>>4)&0xf];

						Vertex vert[4] =
						{
							{ (xx  )*fontWidth, (yy  )*fontHeight, 0.0f, fg, bg, (ch  )*8.0f*texelWidth - texelWidthHalf, utop },
							{ (xx+1)*fontWidth, (yy  )*fontHeight, 0.0f, fg, bg, (ch+1)*8.0f*texelWidth - texelWidthHalf, utop },
							{ (xx+1)*fontWidth, (yy+1)*fontHeight, 0.0f, fg, bg, (ch+1)*8.0f*texelWidth - texelWidthHalf, ubottom },
							{ (xx  )*fontWidth, (yy+1)*fontHeight, 0.0f, fg, bg, (ch  )*8.0f*texelWidth - texelWidthHalf, ubottom },
						};

						base::memCopy(vertex, vert, sizeof(vert) );
						vertex += 4;

						indices[0] = uint16_t(startVertex+0);
						indices[1] = uint16_t(startVertex+1);
						indices[2] = uint16_t(startVertex+2);
						indices[3] = uint16_t(startVertex+2);
						indices[4] = uint16_t(startVertex+3);
						indices[5] = uint16_t(startVertex+0);

						startVertex += 4;
						indices += 6;

						numIndices += 6;
					}

					line++;
				}

				if (numIndices >= numBatchIndices)
				{
					break;
				}
			}

			_renderCtx->blitRender(_blitter, numIndices);
		}
	}

	void ClearQuad::init()
	{
		GRAPHICS_CHECK_API_THREAD();

		if (RendererType::Noop != g_caps.rendererType)
		{
			m_layout
				.begin()
				.add(Attrib::Position, 2, AttribType::Float)
				.end();

			ShaderHandle vsh = createEmbeddedShader(s_embeddedShaders, g_caps.rendererType, "vs_clear");
			BASE_ASSERT(isValid(vsh), "Failed to create clear quad embedded vertex shader \"vs_clear\"");

			for (uint32_t ii = 0, num = g_caps.limits.maxFBAttachments; ii < num; ++ii)
			{
				char name[32];
				base::snprintf(name, BASE_COUNTOF(name), "fs_clear%d", ii);
				ShaderHandle fsh = createEmbeddedShader(s_embeddedShaders, g_caps.rendererType, name);
				BASE_ASSERT(isValid(fsh), "Failed to create clear quad embedded fragment shader \"%s\"", name);

				m_program[ii] = createProgram(vsh, fsh);
				BASE_ASSERT(isValid(m_program[ii]), "Failed to create clear quad program.");
				destroy(fsh);
			}

			destroy(vsh);

			struct Vertex
			{
				float m_x;
				float m_y;
			};

			const uint16_t stride = m_layout.m_stride;
			const graphics::Memory* mem = graphics::alloc(4 * stride);
			Vertex* vertex = (Vertex*)mem->data;
			BASE_ASSERT(stride == sizeof(Vertex), "Stride/Vertex mismatch (stride %d, sizeof(Vertex) %d)", stride, sizeof(Vertex));

			vertex->m_x = -1.0f;
			vertex->m_y = -1.0f;
			vertex++;
			vertex->m_x = 1.0f;
			vertex->m_y = -1.0f;
			vertex++;
			vertex->m_x = -1.0f;
			vertex->m_y = 1.0f;
			vertex++;
			vertex->m_x = 1.0f;
			vertex->m_y = 1.0f;

			m_vb = s_ctx->createVertexBuffer(mem, m_layout, 0);
		}
	}

	void ClearQuad::shutdown()
	{
		GRAPHICS_CHECK_API_THREAD();

		if (RendererType::Noop != g_caps.rendererType)
		{
			for (uint32_t ii = 0, num = g_caps.limits.maxFBAttachments; ii < num; ++ii)
			{
				if (isValid(m_program[ii]) )
				{
					destroy(m_program[ii]);
					m_program[ii].idx = kInvalidHandle;
				}
			}

			s_ctx->destroyVertexBuffer(m_vb);
		}
	}

	const char* s_uniformTypeName[] =
	{
		"sampler1",
		NULL,
		"vec4",
		"mat3",
		"mat4",
	};
	BASE_STATIC_ASSERT(UniformType::Count == BASE_COUNTOF(s_uniformTypeName) );

	const char* getUniformTypeName(UniformType::Enum _enum)
	{
		BASE_ASSERT(_enum < UniformType::Count, "%d < UniformType::Count %d", _enum, UniformType::Count);
		return s_uniformTypeName[_enum];
	}

	UniformType::Enum nameToUniformTypeEnum(const char* _name)
	{
		for (uint32_t ii = 0; ii < UniformType::Count; ++ii)
		{
			if (NULL != s_uniformTypeName[ii]
			&&  0 == base::strCmp(_name, s_uniformTypeName[ii]) )
			{
				return UniformType::Enum(ii);
			}
		}

		return UniformType::Count;
	}

	static const char* s_predefinedName[PredefinedUniform::Count] =
	{
		"u_viewRect",
		"u_viewTexel",
		"u_view",
		"u_invView",
		"u_proj",
		"u_invProj",
		"u_viewProj",
		"u_invViewProj",
		"u_model",
		"u_modelView",
		"u_modelViewProj",
		"u_alphaRef4",
	};

	const char* getPredefinedUniformName(PredefinedUniform::Enum _enum)
	{
		return s_predefinedName[_enum];
	}

	PredefinedUniform::Enum nameToPredefinedUniformEnum(const base::StringView& _name)
	{
		for (uint32_t ii = 0; ii < PredefinedUniform::Count; ++ii)
		{
			if (0 == base::strCmp(_name, s_predefinedName[ii]) )
			{
				return PredefinedUniform::Enum(ii);
			}
		}

		return PredefinedUniform::Count;
	}

	void srtToMatrix4_x1(void* _dst, const void* _src)
	{
		      Matrix4* mtx = reinterpret_cast<  Matrix4*>(_dst);
		const     Srt* srt = reinterpret_cast<const Srt*>(_src);

		const float rx = srt->rotate[0];
		const float ry = srt->rotate[1];
		const float rz = srt->rotate[2];
		const float rw = srt->rotate[3];

		const float xx2 = 2.0f * rx * rx;
		const float yy2 = 2.0f * ry * ry;
		const float zz2 = 2.0f * rz * rz;
		const float yx2 = 2.0f * ry * rx;
		const float yz2 = 2.0f * ry * rz;
		const float yw2 = 2.0f * ry * rw;
		const float wz2 = 2.0f * rw * rz;
		const float wx2 = 2.0f * rw * rx;
		const float xz2 = 2.0f * rx * rz;

		const float sx = srt->scale[0];
		const float sy = srt->scale[1];
		const float sz = srt->scale[2];

		mtx->un.val[ 0] = (1.0f - yy2 - zz2)*sx;
		mtx->un.val[ 1] = (       yx2 + wz2)*sx;
		mtx->un.val[ 2] = (       xz2 - yw2)*sx;
		mtx->un.val[ 3] = 0.0f;

		mtx->un.val[ 4] = (       yx2 - wz2)*sy;
		mtx->un.val[ 5] = (1.0f - xx2 - zz2)*sy;
		mtx->un.val[ 6] = (       yz2 + wx2)*sy;
		mtx->un.val[ 7] = 0.0f;

		mtx->un.val[ 8] = (       xz2 + yw2)*sz;
		mtx->un.val[ 9] = (       yz2 - wx2)*sz;
		mtx->un.val[10] = (1.0f - xx2 - yy2)*sz;
		mtx->un.val[11] = 0.0f;

		const float tx = srt->translate[0];
		const float ty = srt->translate[1];
		const float tz = srt->translate[2];

		mtx->un.val[12] = tx;
		mtx->un.val[13] = ty;
		mtx->un.val[14] = tz;
		mtx->un.val[15] = 1.0f;
	}

	void transpose(void* _dst, uint32_t _dstStride, const void* _src, uint32_t _srcStride = sizeof(base::simd128_t) )
	{
		      uint8_t* dst = reinterpret_cast<      uint8_t *>(_dst);
		const uint8_t* src = reinterpret_cast<const uint8_t *>(_src);

		using namespace base;

		const simd128_t r0 = simd_ld<simd128_t>(src);
		src += _srcStride;

		const simd128_t r1 = simd_ld<simd128_t>(src);
		src += _srcStride;

		const simd128_t r2 = simd_ld<simd128_t>(src);
		src += _srcStride;

		const simd128_t r3 = simd_ld<simd128_t>(src);

		const simd128_t aibj = simd_shuf_xAyB(r0,   r2);   // aibj
		const simd128_t emfn = simd_shuf_xAyB(r1,   r3);   // emfn
		const simd128_t ckdl = simd_shuf_zCwD(r0,   r2);   // ckdl
		const simd128_t gohp = simd_shuf_zCwD(r1,   r3);   // gohp
		const simd128_t aeim = simd_shuf_xAyB(aibj, emfn); // aeim
		const simd128_t bfjn = simd_shuf_zCwD(aibj, emfn); // bfjn
		const simd128_t cgko = simd_shuf_xAyB(ckdl, gohp); // cgko
		const simd128_t dhlp = simd_shuf_zCwD(ckdl, gohp); // dhlp

		simd_st(dst, aeim);
		dst += _dstStride;

		simd_st(dst, bfjn);
		dst += _dstStride;

		simd_st(dst, cgko);
		dst += _dstStride;

		simd_st(dst, dhlp);
	}

	void srtToMatrix4_x4_Ref(void* _dst, const void* _src)
	{
		      uint8_t* dst = reinterpret_cast<      uint8_t*>(_dst);
		const uint8_t* src = reinterpret_cast<const uint8_t*>(_src);

		srtToMatrix4_x1(dst + 0*sizeof(Matrix4), src + 0*sizeof(Srt) );
		srtToMatrix4_x1(dst + 1*sizeof(Matrix4), src + 1*sizeof(Srt) );
		srtToMatrix4_x1(dst + 2*sizeof(Matrix4), src + 2*sizeof(Srt) );
		srtToMatrix4_x1(dst + 3*sizeof(Matrix4), src + 3*sizeof(Srt) );
	}

	void srtToMatrix4_x4_Simd(void* _dst, const void* _src)
	{
		using namespace base;

		      simd128_t* dst = reinterpret_cast<      simd128_t*>(_dst);
		const simd128_t* src = reinterpret_cast<const simd128_t*>(_src);

		simd128_t rotate[4];
		simd128_t translate[4];
		simd128_t scale[4];

		transpose(rotate,    sizeof(simd128_t), src + 0, sizeof(Srt) );
		transpose(translate, sizeof(simd128_t), src + 1, sizeof(Srt) );
		transpose(scale,     sizeof(simd128_t), src + 2, sizeof(Srt) );

		const simd128_t rx    = simd_ld<simd128_t>(rotate + 0);
		const simd128_t ry    = simd_ld<simd128_t>(rotate + 1);
		const simd128_t rz    = simd_ld<simd128_t>(rotate + 2);
		const simd128_t rw    = simd_ld<simd128_t>(rotate + 3);

		const simd128_t tx    = simd_ld<simd128_t>(translate + 0);
		const simd128_t ty    = simd_ld<simd128_t>(translate + 1);
		const simd128_t tz    = simd_ld<simd128_t>(translate + 2);

		const simd128_t sx    = simd_ld<simd128_t>(scale + 0);
		const simd128_t sy    = simd_ld<simd128_t>(scale + 1);
		const simd128_t sz    = simd_ld<simd128_t>(scale + 2);

		const simd128_t zero  = simd_splat(0.0f);
		const simd128_t one   = simd_splat(1.0f);
		const simd128_t two   = simd_splat(2.0f);

		const simd128_t xx    = simd_mul(rx,    rx);
		const simd128_t xx2   = simd_mul(two,   xx);
		const simd128_t yy    = simd_mul(ry,    ry);
		const simd128_t yy2   = simd_mul(two,   yy);
		const simd128_t zz    = simd_mul(rz,    rz);
		const simd128_t zz2   = simd_mul(two,   zz);
		const simd128_t yx    = simd_mul(ry,    rx);
		const simd128_t yx2   = simd_mul(two,   yx);
		const simd128_t yz    = simd_mul(ry,    rz);
		const simd128_t yz2   = simd_mul(two,   yz);
		const simd128_t yw    = simd_mul(ry,    rw);
		const simd128_t yw2   = simd_mul(two,   yw);
		const simd128_t wz    = simd_mul(rw,    rz);
		const simd128_t wz2   = simd_mul(two,   wz);
		const simd128_t wx    = simd_mul(rw,    rx);
		const simd128_t wx2   = simd_mul(two,   wx);
		const simd128_t xz    = simd_mul(rx,    rz);
		const simd128_t xz2   = simd_mul(two,   xz);
		const simd128_t t0x   = simd_sub(one,   yy2);
		const simd128_t r0x   = simd_sub(t0x,   zz2);
		const simd128_t r0y   = simd_add(yx2,   wz2);
		const simd128_t r0z   = simd_sub(xz2,   yw2);
		const simd128_t r1x   = simd_sub(yx2,   wz2);
		const simd128_t omxx2 = simd_sub(one,   xx2);
		const simd128_t r1y   = simd_sub(omxx2, zz2);
		const simd128_t r1z   = simd_add(yz2,   wx2);
		const simd128_t r2x   = simd_add(xz2,   yw2);
		const simd128_t r2y   = simd_sub(yz2,   wx2);
		const simd128_t r2z   = simd_sub(omxx2, yy2);

		simd128_t tmp[4];
		tmp[0] = simd_mul(r0x, sx);
		tmp[1] = simd_mul(r0y, sx);
		tmp[2] = simd_mul(r0z, sx);
		tmp[3] = zero;
		transpose(dst + 0, sizeof(Matrix4), tmp);

		tmp[0] = simd_mul(r1x, sy);
		tmp[1] = simd_mul(r1y, sy);
		tmp[2] = simd_mul(r1z, sy);
		tmp[3] = zero;
		transpose(dst + 1, sizeof(Matrix4), tmp);

		tmp[0] = simd_mul(r2x, sz);
		tmp[1] = simd_mul(r2y, sz);
		tmp[2] = simd_mul(r2z, sz);
		tmp[3] = zero;
		transpose(dst + 2, sizeof(Matrix4), tmp);

		tmp[0] = tx;
		tmp[1] = ty;
		tmp[2] = tz;
		tmp[3] = one;
		transpose(dst + 3, sizeof(Matrix4), tmp);
	}

	void srtToMatrix4(void* _dst, const void* _src, uint32_t _num)
	{
		      uint8_t* dst = reinterpret_cast<      uint8_t*>(_dst);
		const uint8_t* src = reinterpret_cast<const uint8_t*>(_src);

		if (!base::isAligned(src, 16) )
		{
			for (uint32_t ii = 0, num = _num / 4; ii < num; ++ii)
			{
				srtToMatrix4_x4_Ref(dst, src);
				src += 4*sizeof(Srt);
				dst += 4*sizeof(Matrix4);
			}
		}
		else
		{
			for (uint32_t ii = 0, num = _num / 4; ii < num; ++ii)
			{
				srtToMatrix4_x4_Simd(dst, src);
				src += 4*sizeof(Srt);
				dst += 4*sizeof(Matrix4);
			}
		}

		for (uint32_t ii = 0, num = _num & 3; ii < num; ++ii)
		{
			srtToMatrix4_x1(dst, src);
			src += sizeof(Srt);
			dst += sizeof(Matrix4);
		}
	}

	void EncoderImpl::submit(ViewId _id, ProgramHandle _program, OcclusionQueryHandle _occlusionQuery, uint32_t _depth, uint8_t _flags)
	{
		if (BASE_ENABLED(GRAPHICS_CONFIG_DEBUG_UNIFORM)
		&& (_flags & GRAPHICS_DISCARD_STATE))
		{
			m_uniformSet.clear();
		}

		if (BASE_ENABLED(GRAPHICS_CONFIG_DEBUG_OCCLUSION)
		&&  isValid(_occlusionQuery) )
		{
			BASE_ASSERT(m_occlusionQuerySet.end() == m_occlusionQuerySet.find(_occlusionQuery.idx)
				, "OcclusionQuery %d was already used for this frame."
				, _occlusionQuery.idx
				);
			m_occlusionQuerySet.insert(_occlusionQuery.idx);
		}

		if (m_discard)
		{
			discard(_flags);
			return;
		}

		if (0 == m_draw.m_numVertices
		&&  0 == m_draw.m_numIndices)
		{
			discard(_flags);
			++m_numDropped;
			return;
		}

		const uint32_t renderItemIdx = base::atomicFetchAndAddsat<uint32_t>(&m_frame->m_numRenderItems, 1, GRAPHICS_CONFIG_MAX_DRAW_CALLS);
		if (GRAPHICS_CONFIG_MAX_DRAW_CALLS <= renderItemIdx)
		{
			discard(_flags);
			++m_numDropped;
			return;
		}

		++m_numSubmitted;

		UniformBuffer* uniformBuffer = m_frame->m_uniformBuffer[m_uniformIdx];
		m_uniformEnd = uniformBuffer->getPos();

		m_key.m_program = isValid(_program)
			? _program
			: ProgramHandle{0}
			;

		m_key.m_view = _id;

		SortKey::Enum type;
		switch (s_ctx->m_view[_id].m_mode)
		{
		case ViewMode::Sequential:      m_key.m_seq   = s_ctx->getSeqIncr(_id); type = SortKey::SortSequence; break;
		case ViewMode::DepthAscending:  m_key.m_depth =            _depth;      type = SortKey::SortDepth;    break;
		case ViewMode::DepthDescending: m_key.m_depth = UINT32_MAX-_depth;      type = SortKey::SortDepth;    break;
		default:                        m_key.m_depth =            _depth;      type = SortKey::SortProgram;  break;
		}

		uint64_t key = m_key.encodeDraw(type);

		m_frame->m_sortKeys[renderItemIdx]   = key;
		m_frame->m_sortValues[renderItemIdx] = RenderItemCount(renderItemIdx);

		m_draw.m_uniformIdx   = m_uniformIdx;
		m_draw.m_uniformBegin = m_uniformBegin;
		m_draw.m_uniformEnd   = m_uniformEnd;

		if (UINT8_MAX != m_draw.m_streamMask)
		{
			uint32_t numVertices = UINT32_MAX;
			for (uint32_t idx = 0, streamMask = m_draw.m_streamMask
				; 0 != streamMask
				; streamMask >>= 1, idx += 1
				)
			{
				const uint32_t ntz = base::uint32_cnttz(streamMask);
				streamMask >>= ntz;
				idx         += ntz;
				numVertices = base::min(numVertices, m_numVertices[idx]);
			}

			m_draw.m_numVertices = numVertices;
		}
		else
		{
			m_draw.m_numVertices = m_numVertices[0];
		}

		if (isValid(_occlusionQuery) )
		{
			m_draw.m_stateFlags |= GRAPHICS_STATE_INTERNAL_OCCLUSION_QUERY;
			m_draw.m_occlusionQuery = _occlusionQuery;
		}

		m_frame->m_renderItem[renderItemIdx].draw = m_draw;
		m_frame->m_renderItemBind[renderItemIdx]  = m_bind;

		m_draw.clear(_flags);
		m_bind.clear(_flags);
		if (_flags & GRAPHICS_DISCARD_STATE)
		{
			m_uniformBegin = m_uniformEnd;
		}
	}

	void EncoderImpl::dispatch(ViewId _id, ProgramHandle _handle, uint32_t _numX, uint32_t _numY, uint32_t _numZ, uint8_t _flags)
	{
		if (BASE_ENABLED(GRAPHICS_CONFIG_DEBUG_UNIFORM) )
		{
			m_uniformSet.clear();
		}

		if (m_discard)
		{
			discard(_flags);
			return;
		}

		const uint32_t renderItemIdx = base::atomicFetchAndAddsat<uint32_t>(&m_frame->m_numRenderItems, 1, GRAPHICS_CONFIG_MAX_DRAW_CALLS);
		if (GRAPHICS_CONFIG_MAX_DRAW_CALLS-1 <= renderItemIdx)
		{
			discard(_flags);
			++m_numDropped;
			return;
		}

		++m_numSubmitted;

		UniformBuffer* uniformBuffer = m_frame->m_uniformBuffer[m_uniformIdx];
		m_uniformEnd = uniformBuffer->getPos();

		m_compute.m_startMatrix = m_draw.m_startMatrix;
		m_compute.m_numMatrices = m_draw.m_numMatrices;
		m_compute.m_numX   = base::max(_numX, 1u);
		m_compute.m_numY   = base::max(_numY, 1u);
		m_compute.m_numZ   = base::max(_numZ, 1u);

		m_key.m_program = _handle;
		m_key.m_depth   = 0;
		m_key.m_view    = _id;
		m_key.m_seq     = s_ctx->getSeqIncr(_id);

		uint64_t key = m_key.encodeCompute();
		m_frame->m_sortKeys[renderItemIdx]   = key;
		m_frame->m_sortValues[renderItemIdx] = RenderItemCount(renderItemIdx);

		m_compute.m_uniformIdx   = m_uniformIdx;
		m_compute.m_uniformBegin = m_uniformBegin;
		m_compute.m_uniformEnd   = m_uniformEnd;
		m_frame->m_renderItem[renderItemIdx].compute = m_compute;
		m_frame->m_renderItemBind[renderItemIdx]     = m_bind;

		m_compute.clear(_flags);
		m_bind.clear(_flags);
		m_uniformBegin = m_uniformEnd;
	}

	void EncoderImpl::blit(ViewId _id, TextureHandle _dst, uint8_t _dstMip, uint16_t _dstX, uint16_t _dstY, uint16_t _dstZ, TextureHandle _src, uint8_t _srcMip, uint16_t _srcX, uint16_t _srcY, uint16_t _srcZ, uint16_t _width, uint16_t _height, uint16_t _depth)
	{
		BASE_WARN(m_frame->m_numBlitItems < GRAPHICS_CONFIG_MAX_BLIT_ITEMS
			, "Exceed number of available blit items per frame. GRAPHICS_CONFIG_MAX_BLIT_ITEMS is %d. Skipping blit."
			, GRAPHICS_CONFIG_MAX_BLIT_ITEMS
			);
		if (m_frame->m_numBlitItems < GRAPHICS_CONFIG_MAX_BLIT_ITEMS)
		{
			uint16_t item = m_frame->m_numBlitItems++;

			BlitItem& bi = m_frame->m_blitItem[item];
			bi.m_srcX    = _srcX;
			bi.m_srcY    = _srcY;
			bi.m_srcZ    = _srcZ;
			bi.m_dstX    = _dstX;
			bi.m_dstY    = _dstY;
			bi.m_dstZ    = _dstZ;
			bi.m_width   = _width;
			bi.m_height  = _height;
			bi.m_depth   = _depth;
			bi.m_srcMip  = _srcMip;
			bi.m_dstMip  = _dstMip;
			bi.m_src     = _src;
			bi.m_dst     = _dst;

			BlitKey key;
			key.m_view = _id;
			key.m_item = item;
			m_frame->m_blitKeys[item] = key.encode();
		}
	}

	void Frame::sort()
	{
		GRAPHICS_PROFILER_SCOPE("graphics/Sort", 0xff2040ff);

		ViewId viewRemap[GRAPHICS_CONFIG_MAX_VIEWS];
		for (uint32_t ii = 0; ii < GRAPHICS_CONFIG_MAX_VIEWS; ++ii)
		{
			viewRemap[m_viewRemap[ii] ] = ViewId(ii);

			View& view = m_view[ii];
			Rect rect(0, 0, uint16_t(m_resolution.width), uint16_t(m_resolution.height) );

			if (isValid(view.m_fbh) )
			{
				const FrameBufferRef& fbr = s_ctx->m_frameBufferRef[view.m_fbh.idx];
				const BackbufferRatio::Enum bbRatio = fbr.m_window
					? BackbufferRatio::Count
					: BackbufferRatio::Enum(s_ctx->m_textureRef[fbr.un.m_th[0].idx].m_bbRatio)
					;

				if (BackbufferRatio::Count != bbRatio)
				{
					getTextureSizeFromRatio(bbRatio, rect.m_width, rect.m_height);
				}
				else
				{
					rect.m_width  = fbr.m_width;
					rect.m_height = fbr.m_height;
				}
			}

			view.m_rect.intersect(rect);

			if (!view.m_scissor.isZero() )
			{
				view.m_scissor.intersect(rect);
			}
		}

		for (uint32_t ii = 0, num = m_numRenderItems; ii < num; ++ii)
		{
			m_sortKeys[ii] = SortKey::remapView(m_sortKeys[ii], viewRemap);
		}

		base::radixSort(m_sortKeys, s_ctx->m_tempKeys, m_sortValues, s_ctx->m_tempValues, m_numRenderItems);

		for (uint32_t ii = 0, num = m_numBlitItems; ii < num; ++ii)
		{
			m_blitKeys[ii] = BlitKey::remapView(m_blitKeys[ii], viewRemap);
		}

		base::radixSort(m_blitKeys, (uint32_t*)&s_ctx->m_tempKeys, m_numBlitItems);
	}

	RenderFrame::Enum renderFrame(int32_t _msecs)
	{
		if (BASE_ENABLED(GRAPHICS_CONFIG_MULTITHREADED) )
		{
			if (s_renderFrameCalled)
			{
				GRAPHICS_CHECK_RENDER_THREAD();
			}

			if (NULL == s_ctx)
			{
				s_renderFrameCalled = true;
				s_threadIndex = ~GRAPHICS_API_THREAD_MAGIC;
				return RenderFrame::NoContext;
			}

			int32_t msecs = -1 == _msecs
				? GRAPHICS_CONFIG_API_SEMAPHORE_TIMEOUT
				: _msecs
				;
			RenderFrame::Enum result = s_ctx->renderFrame(msecs);
			if (RenderFrame::Exiting == result)
			{
				Context* ctx = s_ctx;
				ctx->apiSemWait();
				s_ctx = NULL;
				ctx->renderSemPost();
			}

			return result;
		}

		BASE_ASSERT(false, "This call only makes sense if used with multi-threaded renderer.");
		return RenderFrame::NoContext;
	}

	const uint32_t g_uniformTypeSize[UniformType::Count+1] =
	{
		sizeof(int32_t),
		0,
		4*sizeof(float),
		3*3*sizeof(float),
		4*4*sizeof(float),
		1,
	};

	void UniformBuffer::writeUniform(UniformType::Enum _type, uint16_t _loc, const void* _value, uint16_t _num)
	{
		uint32_t opcode = encodeOpcode(_type, _loc, _num, true);
		write(opcode);
		write(_value, g_uniformTypeSize[_type]*_num);
	}

	void UniformBuffer::writeUniformHandle(UniformType::Enum _type, uint16_t _loc, UniformHandle _handle, uint16_t _num)
	{
		uint32_t opcode = encodeOpcode(_type, _loc, _num, false);
		write(opcode);
		write(&_handle, sizeof(UniformHandle) );
	}

	void UniformBuffer::writeMarker(const char* _marker)
	{
		uint16_t num = (uint16_t)base::strLen(_marker)+1;
		uint32_t opcode = encodeOpcode(graphics::UniformType::Count, 0, num, true);
		write(opcode);
		write(_marker, num);
	}

	struct CapsFlags
	{
		uint64_t m_flag;
		const char* m_str;
	};

	static const CapsFlags s_capsFlags[] =
	{
#define CAPS_FLAGS(_x) { _x, #_x }
		CAPS_FLAGS(GRAPHICS_CAPS_ALPHA_TO_COVERAGE),
		CAPS_FLAGS(GRAPHICS_CAPS_BLEND_INDEPENDENT),
		CAPS_FLAGS(GRAPHICS_CAPS_COMPUTE),
		CAPS_FLAGS(GRAPHICS_CAPS_CONSERVATIVE_RASTER),
		CAPS_FLAGS(GRAPHICS_CAPS_DRAW_INDIRECT),
		CAPS_FLAGS(GRAPHICS_CAPS_FRAGMENT_DEPTH),
		CAPS_FLAGS(GRAPHICS_CAPS_FRAGMENT_ORDERING),
		CAPS_FLAGS(GRAPHICS_CAPS_GRAPHICS_DEBUGGER),
		CAPS_FLAGS(GRAPHICS_CAPS_HDR10),
		CAPS_FLAGS(GRAPHICS_CAPS_HIDPI),
		CAPS_FLAGS(GRAPHICS_CAPS_IMAGE_RW),
		CAPS_FLAGS(GRAPHICS_CAPS_INDEX32),
		CAPS_FLAGS(GRAPHICS_CAPS_INSTANCING),
		CAPS_FLAGS(GRAPHICS_CAPS_OCCLUSION_QUERY),
		CAPS_FLAGS(GRAPHICS_CAPS_RENDERER_MULTITHREADED),
		CAPS_FLAGS(GRAPHICS_CAPS_SWAP_CHAIN),
		CAPS_FLAGS(GRAPHICS_CAPS_TEXTURE_2D_ARRAY),
		CAPS_FLAGS(GRAPHICS_CAPS_TEXTURE_3D),
		CAPS_FLAGS(GRAPHICS_CAPS_TEXTURE_BLIT),
		CAPS_FLAGS(GRAPHICS_CAPS_TRANSPARENT_BACKBUFFER),
		CAPS_FLAGS(GRAPHICS_CAPS_TEXTURE_COMPARE_ALL),
		CAPS_FLAGS(GRAPHICS_CAPS_TEXTURE_COMPARE_LEQUAL),
		CAPS_FLAGS(GRAPHICS_CAPS_TEXTURE_CUBE_ARRAY),
		CAPS_FLAGS(GRAPHICS_CAPS_TEXTURE_DIRECT_ACCESS),
		CAPS_FLAGS(GRAPHICS_CAPS_TEXTURE_READ_BACK),
		CAPS_FLAGS(GRAPHICS_CAPS_VERTEX_ATTRIB_HALF),
		CAPS_FLAGS(GRAPHICS_CAPS_VERTEX_ATTRIB_UINT10),
		CAPS_FLAGS(GRAPHICS_CAPS_VERTEX_ID),
		CAPS_FLAGS(GRAPHICS_CAPS_VIEWPORT_LAYER_ARRAY),
#undef CAPS_FLAGS
	};

	static void dumpCaps()
	{
		BASE_TRACE("");

		if (0 < g_caps.numGPUs)
		{
			BASE_TRACE("Detected GPUs (%d):", g_caps.numGPUs);
			BASE_TRACE("\t +----------------   Index");
			BASE_TRACE("\t |  +-------------   Device ID");
			BASE_TRACE("\t |  |    +--------   Vendor ID");
			for (uint32_t ii = 0; ii < g_caps.numGPUs; ++ii)
			{
				const Caps::GPU& gpu = g_caps.gpu[ii];
				BASE_UNUSED(gpu);

				BASE_TRACE("\t %d: %04x %04x"
					, ii
					, gpu.deviceId
					, gpu.vendorId
					);
			}

			BASE_TRACE("");
		}

		BASE_TRACE("GPU device, Device ID: %04x, Vendor ID: %04x", g_caps.deviceId, g_caps.vendorId);
		BASE_TRACE("");

		RendererType::Enum renderers[RendererType::Count];
		uint8_t num = getSupportedRenderers(BASE_COUNTOF(renderers), renderers);

		BASE_TRACE("Supported renderer backends (%d):", num);
		for (uint32_t ii = 0; ii < num; ++ii)
		{
			BASE_TRACE("\t - %s", getRendererName(renderers[ii]) );
		}

		BASE_TRACE("");
		BASE_TRACE("Sort key masks:");
		BASE_TRACE("\t   View     %016" PRIx64, kSortKeyViewMask);
		BASE_TRACE("\t   Draw bit %016" PRIx64, kSortKeyDrawBit);

		BASE_TRACE("");
		BASE_TRACE("\tD  Type     %016" PRIx64, kSortKeyDrawTypeMask);

		BASE_TRACE("");
		BASE_TRACE("\tD0 Blend    %016" PRIx64, kSortKeyDraw0BlendMask);
		BASE_TRACE("\tD0 Program  %016" PRIx64, kSortKeyDraw0ProgramMask);
		BASE_TRACE("\tD0 Depth    %016" PRIx64, kSortKeyDraw0DepthMask);

		BASE_TRACE("");
		BASE_TRACE("\tD1 Depth    %016" PRIx64, kSortKeyDraw1DepthMask);
		BASE_TRACE("\tD1 Blend    %016" PRIx64, kSortKeyDraw1BlendMask);
		BASE_TRACE("\tD1 Program  %016" PRIx64, kSortKeyDraw1ProgramMask);

		BASE_TRACE("");
		BASE_TRACE("\tD2 Seq      %016" PRIx64, kSortKeyDraw2SeqMask);
		BASE_TRACE("\tD2 Blend    %016" PRIx64, kSortKeyDraw2BlendMask);
		BASE_TRACE("\tD2 Program  %016" PRIx64, kSortKeyDraw2ProgramMask);

		BASE_TRACE("");
		BASE_TRACE("\t C Seq      %016" PRIx64, kSortKeyComputeSeqMask);
		BASE_TRACE("\t C Program  %016" PRIx64, kSortKeyComputeProgramMask);

		BASE_TRACE("");
		BASE_TRACE("Capabilities (renderer %s, vendor 0x%04x, device 0x%04x):"
				, s_ctx->m_renderCtx->getRendererName()
				, g_caps.vendorId
				, g_caps.deviceId
				);
		for (uint32_t ii = 0; ii < BASE_COUNTOF(s_capsFlags); ++ii)
		{
			BASE_TRACE("\t[%c] %s"
				, 0 != (g_caps.supported & s_capsFlags[ii].m_flag) ? 'x' : ' '
				, s_capsFlags[ii].m_str
				);
		}
		BASE_UNUSED(s_capsFlags);

		BASE_TRACE("");
		BASE_TRACE("Limits:");
#define LIMITS(_x) BASE_TRACE("\t%-24s%10d", #_x, g_caps.limits._x)
		LIMITS(maxDrawCalls);
		LIMITS(maxBlits);
		LIMITS(maxTextureSize);
		LIMITS(maxTextureLayers);
		LIMITS(maxViews);
		LIMITS(maxFrameBuffers);
		LIMITS(maxFBAttachments);
		LIMITS(maxPrograms);
		LIMITS(maxShaders);
		LIMITS(maxTextures);
		LIMITS(maxTextureSamplers);
		LIMITS(maxComputeBindings);
		LIMITS(maxVertexLayouts);
		LIMITS(maxVertexStreams);
		LIMITS(maxIndexBuffers);
		LIMITS(maxVertexBuffers);
		LIMITS(maxDynamicIndexBuffers);
		LIMITS(maxDynamicVertexBuffers);
		LIMITS(maxUniforms);
		LIMITS(maxOcclusionQueries);
		LIMITS(maxEncoders);
		LIMITS(minResourceCbSize);
		LIMITS(transientVbSize);
		LIMITS(transientIbSize);
#undef LIMITS

		BASE_TRACE("");
		BASE_TRACE("Supported texture formats:");
		BASE_TRACE("\t +----------------   2D: x = supported / * = emulated");
		BASE_TRACE("\t |+---------------   2D: sRGB format");
		BASE_TRACE("\t ||+--------------   3D: x = supported / * = emulated");
		BASE_TRACE("\t |||+-------------   3D: sRGB format");
		BASE_TRACE("\t ||||+------------ Cube: x = supported / * = emulated");
		BASE_TRACE("\t |||||+----------- Cube: sRGB format");
		BASE_TRACE("\t ||||||+---------- vertex format");
		BASE_TRACE("\t |||||||+--------- image: i = read-write / r = read / w = write");
		BASE_TRACE("\t ||||||||+-------- framebuffer");
		BASE_TRACE("\t |||||||||+------- MSAA framebuffer");
		BASE_TRACE("\t ||||||||||+------ MSAA texture");
		BASE_TRACE("\t |||||||||||+----- Auto-generated mips");
		BASE_TRACE("\t ||||||||||||  +-- name");
		for (uint32_t ii = 0; ii < TextureFormat::Count; ++ii)
		{
			if (TextureFormat::Unknown != ii
			&&  TextureFormat::UnknownDepth != ii)
			{
				uint32_t flags = g_caps.formats[ii];
				BASE_TRACE("\t[%c%c%c%c%c%c%c%c%c%c%c%c] %s"
					, flags&GRAPHICS_CAPS_FORMAT_TEXTURE_2D               ? 'x' : flags&GRAPHICS_CAPS_FORMAT_TEXTURE_2D_EMULATED ? '*' : ' '
					, flags&GRAPHICS_CAPS_FORMAT_TEXTURE_2D_SRGB          ? 'l' : ' '
					, flags&GRAPHICS_CAPS_FORMAT_TEXTURE_3D               ? 'x' : flags&GRAPHICS_CAPS_FORMAT_TEXTURE_3D_EMULATED ? '*' : ' '
					, flags&GRAPHICS_CAPS_FORMAT_TEXTURE_3D_SRGB          ? 'l' : ' '
					, flags&GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE             ? 'x' : flags&GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE_EMULATED ? '*' : ' '
					, flags&GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE_SRGB        ? 'l' : ' '
					, flags&GRAPHICS_CAPS_FORMAT_TEXTURE_VERTEX           ? 'v' : ' '
					, (flags&GRAPHICS_CAPS_FORMAT_TEXTURE_IMAGE_READ) &&
					  (flags&GRAPHICS_CAPS_FORMAT_TEXTURE_IMAGE_WRITE)    ? 'i' : flags&GRAPHICS_CAPS_FORMAT_TEXTURE_IMAGE_READ ? 'r' : flags&GRAPHICS_CAPS_FORMAT_TEXTURE_IMAGE_WRITE ? 'w' : ' '
					, flags&GRAPHICS_CAPS_FORMAT_TEXTURE_FRAMEBUFFER      ? 'f' : ' '
					, flags&GRAPHICS_CAPS_FORMAT_TEXTURE_FRAMEBUFFER_MSAA ? '+' : ' '
					, flags&GRAPHICS_CAPS_FORMAT_TEXTURE_MSAA             ? 'm' : ' '
					, flags&GRAPHICS_CAPS_FORMAT_TEXTURE_MIP_AUTOGEN      ? 'M' : ' '
					, getName(TextureFormat::Enum(ii) )
					);
				BASE_UNUSED(flags);
			}
		}

		BASE_TRACE("");
		BASE_TRACE("NDC depth [%d, 1], origin %s left."
			, g_caps.homogeneousDepth ? -1 : 0
			, g_caps.originBottomLeft ? "bottom" : "top"
			);

		BASE_TRACE("");
	}

	void dump(const Resolution& _resolution)
	{
		const uint32_t reset = _resolution.reset;
		const uint32_t msaa = (reset&GRAPHICS_RESET_MSAA_MASK)>>GRAPHICS_RESET_MSAA_SHIFT;
		BASE_UNUSED(reset, msaa);

		BASE_TRACE("Reset back-buffer swap chain:");
		BASE_TRACE("\t%dx%d, format: %s, numBackBuffers: %d, maxFrameLatency: %d"
			, _resolution.width
			, _resolution.height
			, TextureFormat::Count == _resolution.format
				? "*default*"
				: bimg::getName(bimg::TextureFormat::Enum(_resolution.format) )
			, _resolution.numBackBuffers
			, _resolution.maxFrameLatency
			);

		BASE_TRACE("\t[%c] MSAAx%d",                 0 != msaa                                        ? 'x' : ' ', 1<<msaa);
		BASE_TRACE("\t[%c] Fullscreen",              0 != (reset & GRAPHICS_RESET_FULLSCREEN)             ? 'x' : ' ');
		BASE_TRACE("\t[%c] V-sync",                  0 != (reset & GRAPHICS_RESET_VSYNC)                  ? 'x' : ' ');
		BASE_TRACE("\t[%c] Max Anisotropy",          0 != (reset & GRAPHICS_RESET_MAXANISOTROPY)          ? 'x' : ' ');
		BASE_TRACE("\t[%c] Capture",                 0 != (reset & GRAPHICS_RESET_CAPTURE)                ? 'x' : ' ');
		BASE_TRACE("\t[%c] Flush After Render",      0 != (reset & GRAPHICS_RESET_FLUSH_AFTER_RENDER)     ? 'x' : ' ');
		BASE_TRACE("\t[%c] Flip After Render",       0 != (reset & GRAPHICS_RESET_FLIP_AFTER_RENDER)      ? 'x' : ' ');
		BASE_TRACE("\t[%c] sRGB Back Buffer",        0 != (reset & GRAPHICS_RESET_SRGB_BACKBUFFER)        ? 'x' : ' ');
		BASE_TRACE("\t[%c] Transparent Back Buffer", 0 != (reset & GRAPHICS_RESET_TRANSPARENT_BACKBUFFER) ? 'x' : ' ');
		BASE_TRACE("\t[%c] HDR10",                   0 != (reset & GRAPHICS_RESET_HDR10)                  ? 'x' : ' ');
		BASE_TRACE("\t[%c] Hi-DPI",                  0 != (reset & GRAPHICS_RESET_HIDPI)                  ? 'x' : ' ');
		BASE_TRACE("\t[%c] Depth Clamp",             0 != (reset & GRAPHICS_RESET_DEPTH_CLAMP)            ? 'x' : ' ');
		BASE_TRACE("\t[%c] Suspend",                 0 != (reset & GRAPHICS_RESET_SUSPEND)                ? 'x' : ' ');
	}

	TextureFormat::Enum getViableTextureFormat(const bimg::ImageContainer& _imageContainer)
	{
		const uint32_t formatCaps = g_caps.formats[_imageContainer.m_format];
		bool convert = 0 == formatCaps;

		if (_imageContainer.m_cubeMap)
		{
			convert |= 0 == (formatCaps & GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE)
					&& 0 != (formatCaps & GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE_EMULATED)
					;
		}
		else if (_imageContainer.m_depth > 1)
		{
			convert |= 0 == (formatCaps & GRAPHICS_CAPS_FORMAT_TEXTURE_3D)
					&& 0 != (formatCaps & GRAPHICS_CAPS_FORMAT_TEXTURE_3D_EMULATED)
					;
		}
		else
		{
			convert |= 0 == (formatCaps & GRAPHICS_CAPS_FORMAT_TEXTURE_2D)
					&& 0 != (formatCaps & GRAPHICS_CAPS_FORMAT_TEXTURE_2D_EMULATED)
					;
		}

		if (convert)
		{
			return TextureFormat::BGRA8;
		}

		return TextureFormat::Enum(_imageContainer.m_format);
	}

	const char* getName(TextureFormat::Enum _fmt)
	{
		return bimg::getName(bimg::TextureFormat::Enum(_fmt));
	}

	const char* getName(UniformHandle _handle)
	{
		return s_ctx->m_uniformRef[_handle.idx].m_name.getPtr();
	}

	const char* getName(ShaderHandle _handle)
	{
		return s_ctx->m_shaderRef[_handle.idx].m_name.getPtr();
	}

	static const char* s_topologyName[] =
	{
		"Triangles",
		"TriStrip",
		"Lines",
		"LineStrip",
		"Points",
	};
	BASE_STATIC_ASSERT(Topology::Count == BASE_COUNTOF(s_topologyName) );

	const char* getName(Topology::Enum _topology)
	{
		return s_topologyName[base::min(_topology, Topology::PointList)];
	}

	const char* getShaderTypeName(uint32_t _magic)
	{
		if (isShaderType(_magic, 'C') )
		{
			return "Compute";
		}
		else if (isShaderType(_magic, 'F') )
		{
			return "Fragment";
		}
		else if (isShaderType(_magic, 'V') )
		{
			return "Vertex";
		}

		BASE_ASSERT(false, "Invalid shader type!");

		return NULL;
	}

	static TextureFormat::Enum s_emulatedFormats[] =
	{
		TextureFormat::BC1,
		TextureFormat::BC2,
		TextureFormat::BC3,
		TextureFormat::BC4,
		TextureFormat::BC5,
		TextureFormat::ETC1,
		TextureFormat::ETC2,
		TextureFormat::ETC2A,
		TextureFormat::ETC2A1,
		TextureFormat::PTC12,
		TextureFormat::PTC14,
		TextureFormat::PTC12A,
		TextureFormat::PTC14A,
		TextureFormat::PTC22,
		TextureFormat::PTC24,
		TextureFormat::ATC,
		TextureFormat::ATCE,
		TextureFormat::ATCI,
		TextureFormat::ASTC4x4,
		TextureFormat::ASTC5x4,
		TextureFormat::ASTC5x5,
		TextureFormat::ASTC6x5,
		TextureFormat::ASTC6x6,
		TextureFormat::ASTC8x5,
		TextureFormat::ASTC8x6,
		TextureFormat::ASTC8x8,
		TextureFormat::ASTC10x5,
		TextureFormat::ASTC10x6,
		TextureFormat::ASTC10x8,
		TextureFormat::ASTC10x10,
		TextureFormat::ASTC12x10,
		TextureFormat::ASTC12x12,
		TextureFormat::BGRA8, // GL doesn't support BGRA8 without extensions.
		TextureFormat::RGBA8, // D3D9 doesn't support RGBA8
	};

	bool Context::init(const Init& _init)
	{
		if (m_rendererInitialized)
		{
			BASE_TRACE("Already initialized!");
			return false;
		}

		m_headless = true
			&&  RendererType::Noop != _init.type
			&&  NULL == _init.platformData.ndt
			&&  NULL == _init.platformData.nwh
			&&  NULL == _init.platformData.context
			&&  NULL == _init.platformData.backBuffer
			&&  NULL == _init.platformData.backBufferDS
			;
		BASE_WARN(!m_headless, "graphics platform data like window handle or backbuffer is not set, creating headless device.");

		if (m_headless
		&&  0 != _init.resolution.width
		&&  0 != _init.resolution.height)
		{
			BASE_TRACE("Initializing headless mode, resolution of non-existing backbuffer can't be larger than 0x0!");
			return false;
		}

		m_init = _init;
		m_init.resolution.reset &= ~GRAPHICS_RESET_INTERNAL_FORCE;
		m_init.resolution.numBackBuffers  = base::clamp<uint8_t>(_init.resolution.numBackBuffers, 2, GRAPHICS_CONFIG_MAX_BACK_BUFFERS);
		m_init.resolution.maxFrameLatency =   base::min<uint8_t>(_init.resolution.maxFrameLatency,   GRAPHICS_CONFIG_MAX_FRAME_LATENCY);
		m_init.resolution.debugTextScale  = base::clamp<uint8_t>(_init.resolution.debugTextScale, 1, GRAPHICS_CONFIG_DEBUG_TEXT_MAX_SCALE);
		dump(m_init.resolution);

		base::memCopy(&g_platformData, &m_init.platformData, sizeof(PlatformData) );

		m_exit    = false;
		m_flipped = true;
		m_debug   = GRAPHICS_DEBUG_NONE;
		m_frameTimeLast = base::getHPCounter();
		m_flipAfterRender = !!(m_init.resolution.reset & GRAPHICS_RESET_FLIP_AFTER_RENDER);

		m_submit->create(_init.limits.minResourceCbSize);

#if GRAPHICS_CONFIG_MULTITHREADED
		m_render->create(_init.limits.minResourceCbSize);

		if (s_renderFrameCalled)
		{
			// When graphics::renderFrame is called before init render thread
			// should not be created.
			BASE_TRACE("Application called graphics::renderFrame directly, not creating render thread.");
			m_singleThreaded = true
				&& ~GRAPHICS_API_THREAD_MAGIC == s_threadIndex
				;
		}
		else
		{
			BASE_TRACE("Creating rendering thread.");
			m_thread.init(renderThread, this, 0, "graphics - renderer backend thread");
			m_singleThreaded = false;
		}
#else
		BASE_TRACE("Multithreaded renderer is disabled.");
		m_singleThreaded = true;
#endif // GRAPHICS_CONFIG_MULTITHREADED

		BASE_TRACE("Running in %s-threaded mode", m_singleThreaded ? "single" : "multi");

		s_threadIndex = GRAPHICS_API_THREAD_MAGIC;

		for (uint32_t ii = 0; ii < BASE_COUNTOF(m_viewRemap); ++ii)
		{
			m_viewRemap[ii] = ViewId(ii);
		}

		for (uint32_t ii = 0; ii < GRAPHICS_CONFIG_MAX_VIEWS; ++ii)
		{
			resetView(ViewId(ii) );
		}

		for (uint32_t ii = 0; ii < BASE_COUNTOF(m_clearColor); ++ii)
		{
			m_clearColor[ii][0] = 0.0f;
			m_clearColor[ii][1] = 0.0f;
			m_clearColor[ii][2] = 0.0f;
			m_clearColor[ii][3] = 1.0f;
		}

		m_vertexLayoutRef.init();

		CommandBuffer& cmdbuf = getCommandBuffer(CommandBuffer::RendererInit);
		cmdbuf.write(_init);

		frameNoRenderWait();

		m_encoderHandle = base::createHandleAlloc(g_allocator, _init.limits.maxEncoders);
		m_encoder       = (EncoderImpl*)base::alignedAlloc(g_allocator, sizeof(EncoderImpl)*_init.limits.maxEncoders, BASE_ALIGNOF(EncoderImpl) );
		m_encoderStats  = (EncoderStats*)base::alloc(g_allocator, sizeof(EncoderStats)*_init.limits.maxEncoders);
		for (uint32_t ii = 0, num = _init.limits.maxEncoders; ii < num; ++ii)
		{
			BASE_PLACEMENT_NEW(&m_encoder[ii], EncoderImpl);
		}

		uint16_t idx = m_encoderHandle->alloc();
		BASE_ASSERT(0 == idx, "Internal encoder handle is not 0 (idx %d).", idx); BASE_UNUSED(idx);
		m_encoder[0].begin(m_submit, 0);
		m_encoder0 = BASE_ENABLED(GRAPHICS_CONFIG_ENCODER_API_ONLY)
			? NULL
			: reinterpret_cast<Encoder*>(&m_encoder[0])
			;

		// Make sure renderer init is called from render thread.
		// g_caps is initialized and available after this point.
		frame();

		if (!m_rendererInitialized)
		{
			getCommandBuffer(CommandBuffer::RendererShutdownEnd);
			frame();
			frame();
			m_vertexLayoutRef.shutdown(m_layoutHandle);
			m_submit->destroy();
#if GRAPHICS_CONFIG_MULTITHREADED
			m_render->destroy();
#endif // GRAPHICS_CONFIG_MULTITHREADED
			return false;
		}

		for (uint32_t ii = 0; ii < BASE_COUNTOF(s_emulatedFormats); ++ii)
		{
			const uint32_t fmt = s_emulatedFormats[ii];
			g_caps.formats[fmt] |= 0 == (g_caps.formats[fmt] & GRAPHICS_CAPS_FORMAT_TEXTURE_2D  ) ? GRAPHICS_CAPS_FORMAT_TEXTURE_2D_EMULATED   : 0;
			g_caps.formats[fmt] |= 0 == (g_caps.formats[fmt] & GRAPHICS_CAPS_FORMAT_TEXTURE_3D  ) ? GRAPHICS_CAPS_FORMAT_TEXTURE_3D_EMULATED   : 0;
			g_caps.formats[fmt] |= 0 == (g_caps.formats[fmt] & GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE) ? GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE_EMULATED : 0;
		}

		for (uint32_t ii = 0; ii < TextureFormat::UnknownDepth; ++ii)
		{
			bool convertable = bimg::imageConvert(bimg::TextureFormat::BGRA8, bimg::TextureFormat::Enum(ii) );
			g_caps.formats[ii] |= 0 == (g_caps.formats[ii] & GRAPHICS_CAPS_FORMAT_TEXTURE_2D  ) && convertable ? GRAPHICS_CAPS_FORMAT_TEXTURE_2D_EMULATED   : 0;
			g_caps.formats[ii] |= 0 == (g_caps.formats[ii] & GRAPHICS_CAPS_FORMAT_TEXTURE_3D  ) && convertable ? GRAPHICS_CAPS_FORMAT_TEXTURE_3D_EMULATED   : 0;
			g_caps.formats[ii] |= 0 == (g_caps.formats[ii] & GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE) && convertable ? GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE_EMULATED : 0;
		}

		g_caps.rendererType = m_renderCtx->getRendererType();
		initAttribTypeSizeTable(g_caps.rendererType);

		g_caps.supported &= _init.capabilities;
		g_caps.supported |= 0
			| (BASE_ENABLED(GRAPHICS_CONFIG_MULTITHREADED) && !m_singleThreaded ? GRAPHICS_CAPS_RENDERER_MULTITHREADED : 0)
			| (isGraphicsDebuggerPresent() ? GRAPHICS_CAPS_GRAPHICS_DEBUGGER : 0)
			;

		dumpCaps();

		m_textVideoMemBlitter.init(m_init.resolution.debugTextScale);
		m_clearQuad.init();

		m_submit->m_transientVb = createTransientVertexBuffer(_init.limits.transientVbSize);
		m_submit->m_transientIb = createTransientIndexBuffer(_init.limits.transientIbSize);
		frame();

		if (BASE_ENABLED(GRAPHICS_CONFIG_MULTITHREADED) )
		{
			m_submit->m_transientVb = createTransientVertexBuffer(_init.limits.transientVbSize);
			m_submit->m_transientIb = createTransientIndexBuffer(_init.limits.transientIbSize);
			frame();
		}

		g_internalData.caps = getCaps();

		return true;
	}

	void Context::shutdown()
	{
		getCommandBuffer(CommandBuffer::RendererShutdownBegin);
		frame();

		destroyTransientVertexBuffer(m_submit->m_transientVb);
		destroyTransientIndexBuffer(m_submit->m_transientIb);
		m_textVideoMemBlitter.shutdown();
		m_clearQuad.shutdown();
		frame();

		if (BASE_ENABLED(GRAPHICS_CONFIG_MULTITHREADED) )
		{
			destroyTransientVertexBuffer(m_submit->m_transientVb);
			destroyTransientIndexBuffer(m_submit->m_transientIb);
			frame();
		}

		frame(); // If any VertexLayouts needs to be destroyed.

		getCommandBuffer(CommandBuffer::RendererShutdownEnd);
		frame();

		m_encoder[0].end(true);
		m_encoderHandle->free(0);
		base::destroyHandleAlloc(g_allocator, m_encoderHandle);
		m_encoderHandle = NULL;

		for (uint32_t ii = 0, num = g_caps.limits.maxEncoders; ii < num; ++ii)
		{
			m_encoder[ii].~EncoderImpl();
		}

		base::alignedFree(g_allocator, m_encoder, BASE_ALIGNOF(EncoderImpl) );
		base::free(g_allocator, m_encoderStats);

		m_dynVertexBufferAllocator.compact();
		m_dynIndexBufferAllocator.compact();

		BASE_ASSERT(
			  m_layoutHandle.getNumHandles() == m_vertexLayoutRef.m_vertexLayoutMap.getNumElements()
			, "VertexLayoutRef mismatch, num handles %d, handles in hash map %d."
			, m_layoutHandle.getNumHandles()
			, m_vertexLayoutRef.m_vertexLayoutMap.getNumElements()
			);

		m_vertexLayoutRef.shutdown(m_layoutHandle);

#if GRAPHICS_CONFIG_MULTITHREADED
		// Render thread shutdown sequence.
		renderSemWait(); // Wait for previous frame.
		apiSemPost();   // OK to set context to NULL.
		// s_ctx is NULL here.
		renderSemWait(); // In RenderFrame::Exiting state.

		if (m_thread.isRunning() )
		{
			m_thread.shutdown();
		}

		m_render->destroy();
#endif // GRAPHICS_CONFIG_MULTITHREADED

		base::memSet(&g_internalData, 0, sizeof(InternalData) );
		s_ctx = NULL;

		m_submit->destroy();

		if (BASE_ENABLED(GRAPHICS_CONFIG_DEBUG) )
		{
#define CHECK_HANDLE_LEAK(_name, _handleAlloc)                                        \
	BASE_MACRO_BLOCK_BEGIN                                                              \
		if (0 != _handleAlloc.getNumHandles() )                                       \
		{                                                                             \
			BASE_TRACE("LEAK: %s %d (max: %d)"                                          \
				, _name                                                               \
				, _handleAlloc.getNumHandles()                                        \
				, _handleAlloc.getMaxHandles()                                        \
				);                                                                    \
			for (uint16_t ii = 0, num = _handleAlloc.getNumHandles(); ii < num; ++ii) \
			{                                                                         \
				BASE_TRACE("\t%3d: %4d", ii, _handleAlloc.getHandleAt(ii) );            \
			}                                                                         \
		}                                                                             \
	BASE_MACRO_BLOCK_END

#define CHECK_HANDLE_LEAK_NAME(_name, _handleAlloc, _type, _ref)                      \
	BASE_MACRO_BLOCK_BEGIN                                                              \
		if (0 != _handleAlloc.getNumHandles() )                                       \
		{                                                                             \
			BASE_TRACE("LEAK: %s %d (max: %d)"                                          \
				, _name                                                               \
				, _handleAlloc.getNumHandles()                                        \
				, _handleAlloc.getMaxHandles()                                        \
				);                                                                    \
			for (uint16_t ii = 0, num = _handleAlloc.getNumHandles(); ii < num; ++ii) \
			{                                                                         \
				uint16_t idx = _handleAlloc.getHandleAt(ii);                          \
				const _type& ref = _ref[idx]; BASE_UNUSED(ref);                         \
				BASE_TRACE("\t%3d: %4d %s"                                              \
					, ii                                                              \
					, idx                                                             \
					, ref.m_name.getPtr()                                             \
					);                                                                \
			}                                                                         \
		}                                                                             \
	BASE_MACRO_BLOCK_END

#define CHECK_HANDLE_LEAK_RC_NAME(_name, _handleAlloc, _type, _ref)                   \
	BASE_MACRO_BLOCK_BEGIN                                                              \
		if (0 != _handleAlloc.getNumHandles() )                                       \
		{                                                                             \
			BASE_TRACE("LEAK: %s %d (max: %d)"                                          \
				, _name                                                               \
				, _handleAlloc.getNumHandles()                                        \
				, _handleAlloc.getMaxHandles()                                        \
				);                                                                    \
			for (uint16_t ii = 0, num = _handleAlloc.getNumHandles(); ii < num; ++ii) \
			{                                                                         \
				uint16_t idx = _handleAlloc.getHandleAt(ii);                          \
				const _type& ref = _ref[idx]; BASE_UNUSED(ref);                         \
				BASE_TRACE("\t%3d: %4d %s (count %d)"                                   \
					, ii                                                              \
					, idx                                                             \
					, ref.m_name.getPtr()                                             \
					, ref.m_refCount                                                  \
					);                                                                \
			}                                                                         \
		}                                                                             \
	BASE_MACRO_BLOCK_END

			CHECK_HANDLE_LEAK        ("DynamicIndexBufferHandle",  m_dynamicIndexBufferHandle                                  );
			CHECK_HANDLE_LEAK        ("DynamicVertexBufferHandle", m_dynamicVertexBufferHandle                                 );
			CHECK_HANDLE_LEAK_NAME   ("IndexBufferHandle",         m_indexBufferHandle,        IndexBuffer,    m_indexBuffers  );
			CHECK_HANDLE_LEAK        ("VertexLayoutHandle",        m_layoutHandle                                              );
			CHECK_HANDLE_LEAK_NAME   ("VertexBufferHandle",        m_vertexBufferHandle,       VertexBuffer,   m_vertexBuffers );
			CHECK_HANDLE_LEAK_RC_NAME("ShaderHandle",              m_shaderHandle,             ShaderRef,      m_shaderRef     );
			CHECK_HANDLE_LEAK        ("ProgramHandle",             m_programHandle                                             );
			CHECK_HANDLE_LEAK_RC_NAME("TextureHandle",             m_textureHandle,            TextureRef,     m_textureRef    );
			CHECK_HANDLE_LEAK_NAME   ("FrameBufferHandle",         m_frameBufferHandle,        FrameBufferRef, m_frameBufferRef);
			CHECK_HANDLE_LEAK_RC_NAME("UniformHandle",             m_uniformHandle,            UniformRef,     m_uniformRef    );
			CHECK_HANDLE_LEAK        ("OcclusionQueryHandle",      m_occlusionQueryHandle                                      );
#undef CHECK_HANDLE_LEAK
#undef CHECK_HANDLE_LEAK_NAME
		}
	}

	void Context::freeDynamicBuffers()
	{
		for (uint16_t ii = 0, num = m_numFreeDynamicIndexBufferHandles; ii < num; ++ii)
		{
			destroyDynamicIndexBufferInternal(m_freeDynamicIndexBufferHandle[ii]);
		}
		m_numFreeDynamicIndexBufferHandles = 0;

		for (uint16_t ii = 0, num = m_numFreeDynamicVertexBufferHandles; ii < num; ++ii)
		{
			destroyDynamicVertexBufferInternal(m_freeDynamicVertexBufferHandle[ii]);
		}
		m_numFreeDynamicVertexBufferHandles = 0;

		for (uint16_t ii = 0, num = m_numFreeOcclusionQueryHandles; ii < num; ++ii)
		{
			m_occlusionQueryHandle.free(m_freeOcclusionQueryHandle[ii].idx);
		}
		m_numFreeOcclusionQueryHandles = 0;
	}

	void Context::freeAllHandles(Frame* _frame)
	{
		for (uint16_t ii = 0, num = _frame->m_freeIndexBuffer.getNumQueued(); ii < num; ++ii)
		{
			m_indexBufferHandle.free(_frame->m_freeIndexBuffer.get(ii).idx);
		}

		for (uint16_t ii = 0, num = _frame->m_freeVertexBuffer.getNumQueued(); ii < num; ++ii)
		{
			destroyVertexBufferInternal(_frame->m_freeVertexBuffer.get(ii));
		}

		for (uint16_t ii = 0, num = _frame->m_freeVertexLayout.getNumQueued(); ii < num; ++ii)
		{
			m_layoutHandle.free(_frame->m_freeVertexLayout.get(ii).idx);
		}

		for (uint16_t ii = 0, num = _frame->m_freeShader.getNumQueued(); ii < num; ++ii)
		{
			m_shaderHandle.free(_frame->m_freeShader.get(ii).idx);
		}

		for (uint16_t ii = 0, num = _frame->m_freeProgram.getNumQueued(); ii < num; ++ii)
		{
			m_programHandle.free(_frame->m_freeProgram.get(ii).idx);
		}

		for (uint16_t ii = 0, num = _frame->m_freeTexture.getNumQueued(); ii < num; ++ii)
		{
			m_textureHandle.free(_frame->m_freeTexture.get(ii).idx);
		}

		for (uint16_t ii = 0, num = _frame->m_freeFrameBuffer.getNumQueued(); ii < num; ++ii)
		{
			m_frameBufferHandle.free(_frame->m_freeFrameBuffer.get(ii).idx);
		}

		for (uint16_t ii = 0, num = _frame->m_freeUniform.getNumQueued(); ii < num; ++ii)
		{
			m_uniformHandle.free(_frame->m_freeUniform.get(ii).idx);
		}
	}

	Encoder* Context::begin(bool _forThread)
	{
		EncoderImpl* encoder = &m_encoder[0];

#if GRAPHICS_CONFIG_MULTITHREADED
		if (_forThread || GRAPHICS_API_THREAD_MAGIC != s_threadIndex)
		{
			base::MutexScope scopeLock(m_encoderApiLock);

			uint16_t idx = m_encoderHandle->alloc();
			if (kInvalidHandle == idx)
			{
				return NULL;
			}

			encoder = &m_encoder[idx];
			encoder->begin(m_submit, uint8_t(idx) );
		}
#else
		BASE_UNUSED(_forThread);
#endif // GRAPHICS_CONFIG_MULTITHREADED

		return reinterpret_cast<Encoder*>(encoder);
	}

	void Context::end(Encoder* _encoder)
	{
#if GRAPHICS_CONFIG_MULTITHREADED
		EncoderImpl* encoder = reinterpret_cast<EncoderImpl*>(_encoder);
		if (encoder != &m_encoder[0])
		{
			encoder->end(true);
			m_encoderEndSem.post();
		}
#else
		BASE_UNUSED(_encoder);
#endif // GRAPHICS_CONFIG_MULTITHREADED
	}

	uint32_t Context::frame(bool _capture)
	{
		m_encoder[0].end(true);

#if GRAPHICS_CONFIG_MULTITHREADED
		base::MutexScope resourceApiScope(m_resourceApiLock);

		encoderApiWait();
		base::MutexScope encoderApiScope(m_encoderApiLock);
#else
		encoderApiWait();
#endif // GRAPHICS_CONFIG_MULTITHREADED

		m_submit->m_capture = _capture;

		uint32_t frameNum = m_submit->m_frameNum;

		GRAPHICS_PROFILER_SCOPE("graphics/API thread frame", 0xff2040ff);
		// wait for render thread to finish
		renderSemWait();
		frameNoRenderWait();

		m_encoder[0].begin(m_submit, 0);

		return frameNum;
	}

	void Context::frameNoRenderWait()
	{
		swap();

		// release render thread
		apiSemPost();
	}

	void Context::swap()
	{
		freeDynamicBuffers();
		m_submit->m_resolution = m_init.resolution;
		m_init.resolution.reset &= ~GRAPHICS_RESET_INTERNAL_FORCE;
		m_submit->m_debug = m_debug;
		m_submit->m_perfStats.numViews = 0;

		base::memCopy(m_submit->m_viewRemap, m_viewRemap, sizeof(m_viewRemap) );
		base::memCopy(m_submit->m_view, m_view, sizeof(m_view) );

		if (m_colorPaletteDirty > 0)
		{
			--m_colorPaletteDirty;
			base::memCopy(m_submit->m_colorPalette, m_clearColor, sizeof(m_clearColor) );
		}

		freeAllHandles(m_submit);
		m_submit->resetFreeHandles();

		m_submit->finish();

		base::swap(m_render, m_submit);

		base::memCopy(m_render->m_occlusion, m_submit->m_occlusion, sizeof(m_submit->m_occlusion) );

		if (!BASE_ENABLED(GRAPHICS_CONFIG_MULTITHREADED)
		||  m_singleThreaded)
		{
			renderFrame();
		}

		uint32_t nextFrameNum = m_render->m_frameNum + 1;
		m_submit->start(nextFrameNum);

		base::memSet(m_seq, 0, sizeof(m_seq) );

		m_submit->m_textVideoMem->resize(
			  m_render->m_textVideoMem->m_small
			, m_init.resolution.width
			, m_init.resolution.height
			);

		int64_t now = base::getHPCounter();
		m_submit->m_perfStats.cpuTimeFrame = now - m_frameTimeLast;
		m_frameTimeLast = now;
	}

	///
	RendererContextI* rendererCreate(const Init& _init);

	///
	void rendererDestroy(RendererContextI* _renderCtx);

	void Context::flip()
	{
		if (m_rendererInitialized
		&& !m_flipped)
		{
			m_renderCtx->flip();
			m_flipped = true;

			if (m_renderCtx->isDeviceRemoved() )
			{
				// Something horribly went wrong, fallback to noop renderer.
				rendererDestroy(m_renderCtx);

				Init init;
				init.type = RendererType::Noop;
				m_renderCtx = rendererCreate(init);
				g_caps.rendererType = RendererType::Noop;
			}
		}
	}

#if BASE_PLATFORM_OSX || BASE_PLATFORM_IOS
	struct NSAutoreleasePoolScope
	{
		NSAutoreleasePoolScope()
		{
			id obj = class_createInstance(objc_getClass("NSAutoreleasePool"), 0);
			typedef id(*objc_msgSend_init)(void*, SEL);
			pool = ((objc_msgSend_init)objc_msgSend)(obj, sel_getUid("init") );
		}

		~NSAutoreleasePoolScope()
		{
			typedef void(*objc_msgSend_release)(void*, SEL);
			((objc_msgSend_release)objc_msgSend)(pool, sel_getUid("release") );
		}

		id pool;
	};
#endif // BASE_PLATFORM_OSX

	RenderFrame::Enum Context::renderFrame(int32_t _msecs)
	{
		GRAPHICS_PROFILER_SCOPE("graphics::renderFrame", 0xff2040ff);

#if BASE_PLATFORM_OSX || BASE_PLATFORM_IOS
		NSAutoreleasePoolScope pool;
#endif // BASE_PLATFORM_OSX

		if (!m_flipAfterRender)
		{
			GRAPHICS_PROFILER_SCOPE("graphics/flip", 0xff2040ff);
			flip();
		}

		if (apiSemWait(_msecs) )
		{
			{
				GRAPHICS_PROFILER_SCOPE("graphics/Exec commands pre", 0xff2040ff);
				rendererExecCommands(m_render->m_cmdPre);
			}

			if (m_rendererInitialized)
			{
				{
					GRAPHICS_PROFILER_SCOPE("graphics/Render submit", 0xff2040ff);
					m_renderCtx->submit(m_render, m_clearQuad, m_textVideoMemBlitter);
					m_flipped = false;
				}

				{
					GRAPHICS_PROFILER_SCOPE("graphics/Screenshot", 0xff2040ff);
					for (uint8_t ii = 0, num = m_render->m_numScreenShots; ii < num; ++ii)
					{
						const ScreenShot& screenShot = m_render->m_screenShot[ii];
						m_renderCtx->requestScreenShot(screenShot.handle, screenShot.filePath.getCPtr() );
					}
				}
			}

			{
				GRAPHICS_PROFILER_SCOPE("graphics/Exec commands post", 0xff2040ff);
				rendererExecCommands(m_render->m_cmdPost);
			}

			renderSemPost();

			if (m_flipAfterRender)
			{
				GRAPHICS_PROFILER_SCOPE("graphics/flip", 0xff2040ff);
				flip();
			}
		}
		else
		{
			return RenderFrame::Timeout;
		}

		return m_exit
			? RenderFrame::Exiting
			: RenderFrame::Render
			;
	}

	void rendererUpdateUniforms(RendererContextI* _renderCtx, UniformBuffer* _uniformBuffer, uint32_t _begin, uint32_t _end)
	{
		_uniformBuffer->reset(_begin);
		while (_uniformBuffer->getPos() < _end)
		{
			uint32_t opcode = _uniformBuffer->read();

			if (UniformType::End == opcode)
			{
				break;
			}

			UniformType::Enum type;
			uint16_t loc;
			uint16_t num;
			uint16_t copy;
			UniformBuffer::decodeOpcode(opcode, type, loc, num, copy);

			uint32_t size = g_uniformTypeSize[type]*num;
			const char* data = _uniformBuffer->read(size);
			if (UniformType::Count > type)
			{
				if (copy)
				{
					_renderCtx->updateUniform(loc, data, size);
				}
				else
				{
					_renderCtx->updateUniform(loc, *(const char**)(data), size);
				}
			}
			else
			{
				_renderCtx->setMarker(data, uint16_t(size)-1);
			}
		}
	}

	void Context::flushTextureUpdateBatch(CommandBuffer& _cmdbuf)
	{
		if (m_textureUpdateBatch.sort() )
		{
			const uint32_t pos = _cmdbuf.m_pos;

			uint32_t currentKey = UINT32_MAX;

			for (uint32_t ii = 0, num = m_textureUpdateBatch.m_num; ii < num; ++ii)
			{
				_cmdbuf.m_pos = m_textureUpdateBatch.m_values[ii];

				TextureHandle handle;
				_cmdbuf.read(handle);

				uint8_t side;
				_cmdbuf.read(side);

				uint8_t mip;
				_cmdbuf.read(mip);

				Rect rect;
				_cmdbuf.read(rect);

				uint16_t zz;
				_cmdbuf.read(zz);

				uint16_t depth;
				_cmdbuf.read(depth);

				uint16_t pitch;
				_cmdbuf.read(pitch);

				const Memory* mem;
				_cmdbuf.read(mem);

				uint32_t key = m_textureUpdateBatch.m_keys[ii];
				if (key != currentKey)
				{
					if (currentKey != UINT32_MAX)
					{
						m_renderCtx->updateTextureEnd();
					}
					currentKey = key;
					m_renderCtx->updateTextureBegin(handle, side, mip);
				}

				m_renderCtx->updateTexture(handle, side, mip, rect, zz, depth, pitch, mem);

				release(mem);
			}

			if (currentKey != UINT32_MAX)
			{
				m_renderCtx->updateTextureEnd();
			}

			m_textureUpdateBatch.reset();

			_cmdbuf.m_pos = pos;
		}
	}

	typedef RendererContextI* (*RendererCreateFn)(const Init& _init);
	typedef void (*RendererDestroyFn)();

#define GRAPHICS_RENDERER_CONTEXT(_namespace)                           \
	namespace _namespace                                            \
	{                                                               \
		extern RendererContextI* rendererCreate(const Init& _init); \
		extern void rendererDestroy();                              \
	}

	GRAPHICS_RENDERER_CONTEXT(noop);
	GRAPHICS_RENDERER_CONTEXT(agc);
	GRAPHICS_RENDERER_CONTEXT(d3d9);
	GRAPHICS_RENDERER_CONTEXT(d3d11);
	GRAPHICS_RENDERER_CONTEXT(d3d12);
	GRAPHICS_RENDERER_CONTEXT(gnm);
	GRAPHICS_RENDERER_CONTEXT(mtl);
	GRAPHICS_RENDERER_CONTEXT(nvn);
	GRAPHICS_RENDERER_CONTEXT(gl);
	GRAPHICS_RENDERER_CONTEXT(vk);
	GRAPHICS_RENDERER_CONTEXT(webgpu);

#undef GRAPHICS_RENDERER_CONTEXT

	struct RendererCreator
	{
		RendererCreateFn  createFn;
		RendererDestroyFn destroyFn;
		const char* name;
		bool supported;
	};

	static RendererCreator s_rendererCreator[] =
	{
		{ noop::rendererCreate,   noop::rendererDestroy,   GRAPHICS_RENDERER_NOOP_NAME,       true                              }, // Noop
		{ agc::rendererCreate,    agc::rendererDestroy,    GRAPHICS_RENDERER_AGC_NAME,        !!GRAPHICS_CONFIG_RENDERER_AGC        }, // GNM
		{ d3d9::rendererCreate,   d3d9::rendererDestroy,   GRAPHICS_RENDERER_DIRECT3D9_NAME,  !!GRAPHICS_CONFIG_RENDERER_DIRECT3D9  }, // Direct3D9
		{ d3d11::rendererCreate,  d3d11::rendererDestroy,  GRAPHICS_RENDERER_DIRECT3D11_NAME, !!GRAPHICS_CONFIG_RENDERER_DIRECT3D11 }, // Direct3D11
		{ d3d12::rendererCreate,  d3d12::rendererDestroy,  GRAPHICS_RENDERER_DIRECT3D12_NAME, !!GRAPHICS_CONFIG_RENDERER_DIRECT3D12 }, // Direct3D12
		{ gnm::rendererCreate,    gnm::rendererDestroy,    GRAPHICS_RENDERER_GNM_NAME,        !!GRAPHICS_CONFIG_RENDERER_GNM        }, // GNM
#if BASE_PLATFORM_OSX || BASE_PLATFORM_IOS
		{ mtl::rendererCreate,    mtl::rendererDestroy,    GRAPHICS_RENDERER_METAL_NAME,      !!GRAPHICS_CONFIG_RENDERER_METAL      }, // Metal
#else
		{ noop::rendererCreate,   noop::rendererDestroy,   GRAPHICS_RENDERER_NOOP_NAME,       false                             }, // Noop
#endif // BASE_PLATFORM_OSX || BASE_PLATFORM_IOS
		{ nvn::rendererCreate,    nvn::rendererDestroy,    GRAPHICS_RENDERER_NVN_NAME,        !!GRAPHICS_CONFIG_RENDERER_NVN        }, // NVN
		{ gl::rendererCreate,     gl::rendererDestroy,     GRAPHICS_RENDERER_OPENGL_NAME,     !!GRAPHICS_CONFIG_RENDERER_OPENGLES   }, // OpenGLES
		{ gl::rendererCreate,     gl::rendererDestroy,     GRAPHICS_RENDERER_OPENGL_NAME,     !!GRAPHICS_CONFIG_RENDERER_OPENGL     }, // OpenGL
		{ vk::rendererCreate,     vk::rendererDestroy,     GRAPHICS_RENDERER_VULKAN_NAME,     !!GRAPHICS_CONFIG_RENDERER_VULKAN     }, // Vulkan
		{ webgpu::rendererCreate, webgpu::rendererDestroy, GRAPHICS_RENDERER_WEBGPU_NAME,     !!GRAPHICS_CONFIG_RENDERER_WEBGPU     }, // WebGPU
	};
	BASE_STATIC_ASSERT(BASE_COUNTOF(s_rendererCreator) == RendererType::Count);

	bool windowsVersionIs(Condition::Enum _op, uint32_t _version)
	{
#if BASE_PLATFORM_WINDOWS
		static const uint8_t s_condition[] =
		{
			VER_LESS_EQUAL,
			VER_GREATER_EQUAL,
		};

		OSVERSIONINFOEXA ovi;
		base::memSet(&ovi, 0, sizeof(ovi) );
		ovi.dwOSVersionInfoSize = sizeof(ovi);
		// _WIN32_WINNT_WINBLUE 0x0603
		// _WIN32_WINNT_WIN8    0x0602
		// _WIN32_WINNT_WIN7    0x0601
		// _WIN32_WINNT_VISTA   0x0600
		ovi.dwMajorVersion = HIBYTE(_version);
		ovi.dwMinorVersion = LOBYTE(_version);
		DWORDLONG cond = 0;
		VER_SET_CONDITION(cond, VER_MAJORVERSION, s_condition[_op]);
		VER_SET_CONDITION(cond, VER_MINORVERSION, s_condition[_op]);
		return !!VerifyVersionInfoA(&ovi, VER_MAJORVERSION | VER_MINORVERSION, cond);
#else
		BASE_UNUSED(_op, _version);
		return false;
#endif // BASE_PLATFORM_WINDOWS
	}

	RendererContextI* rendererCreate(const Init& _init)
	{
		int32_t scores[RendererType::Count];
		uint32_t numScores = 0;

		for (uint32_t ii = 0; ii < RendererType::Count; ++ii)
		{
			RendererType::Enum renderer = RendererType::Enum(ii);
			if (s_rendererCreator[ii].supported)
			{
				int32_t score = 0;
				if (_init.type == renderer)
				{
					score += 1000;
				}

				score += RendererType::Noop != renderer ? 1 : 0;

				if (BASE_ENABLED(BASE_PLATFORM_WINDOWS) )
				{
					if (windowsVersionIs(Condition::GreaterEqual, 0x0602) )
					{
						score += RendererType::Direct3D11 == renderer ? 20 : 0;
						score += RendererType::Direct3D12 == renderer ? 10 : 0;
					}
					else if (windowsVersionIs(Condition::GreaterEqual, 0x0601) )
					{
						score += RendererType::Direct3D11 == renderer ?   20 : 0;
						score += RendererType::Direct3D9  == renderer ?   10 : 0;
						score += RendererType::Direct3D12 == renderer ? -100 : 0;
					}
					else
					{
						score += RendererType::Direct3D12 == renderer ? -100 : 0;
					}
				}
				else if (BASE_ENABLED(BASE_PLATFORM_LINUX) )
				{
					score += RendererType::Vulkan     == renderer ? 50 : 0;
					score += RendererType::OpenGL     == renderer ? 40 : 0;
					score += RendererType::OpenGLES   == renderer ? 30 : 0;
					score += RendererType::Direct3D12 == renderer ? 20 : 0;
					score += RendererType::Direct3D11 == renderer ? 10 : 0;
					score += RendererType::Direct3D9  == renderer ?  5 : 0;
				}
				else if (BASE_ENABLED(BASE_PLATFORM_OSX) )
				{
					score += RendererType::Metal    == renderer ? 20 : 0;
					score += RendererType::OpenGL   == renderer ? 10 : 0;
				}
				else if (BASE_ENABLED(BASE_PLATFORM_IOS) )
				{
					score += RendererType::Metal    == renderer ? 20 : 0;
					score += RendererType::OpenGLES == renderer ? 10 : 0;
				}
				else if (BASE_ENABLED(0
					 ||  BASE_PLATFORM_ANDROID
					 ||  BASE_PLATFORM_EMSCRIPTEN
					 ||  BASE_PLATFORM_RPI
					 ) )
				{
					score += RendererType::OpenGLES == renderer ? 20 : 0;
				}
				else if (BASE_ENABLED(BASE_PLATFORM_PS4) )
				{
					score += RendererType::Gnm      == renderer ? 20 : 0;
				}
				else if (BASE_ENABLED(0
					 ||  BASE_PLATFORM_XBOXONE
					 ||  BASE_PLATFORM_WINRT
					 ) )
				{
					score += RendererType::Direct3D12 == renderer ? 20 : 0;
					score += RendererType::Direct3D11 == renderer ? 10 : 0;
				}

				scores[numScores++] = (score<<8) | uint8_t(renderer);
			}
		}

		base::quickSort(scores, numScores, base::compareDescending<int32_t>);

		RendererContextI* renderCtx = NULL;
		for (uint32_t ii = 0; ii < numScores; ++ii)
		{
			RendererType::Enum renderer = RendererType::Enum(scores[ii] & 0xff);
			renderCtx = s_rendererCreator[renderer].createFn(_init);
			if (NULL != renderCtx)
			{
				break;
			}

			s_rendererCreator[renderer].supported = false;
		}

		return renderCtx;
	}

	void rendererDestroy(RendererContextI* _renderCtx)
	{
		if (NULL != _renderCtx)
		{
			s_rendererCreator[_renderCtx->getRendererType()].destroyFn();
		}
	}

	void Context::rendererExecCommands(CommandBuffer& _cmdbuf)
	{
		_cmdbuf.reset();

		bool end = false;

		if (NULL == m_renderCtx)
		{
			uint8_t command;
			_cmdbuf.read(command);

			switch (command)
			{
			case CommandBuffer::RendererShutdownEnd:
				m_exit = true;
				return;

			case CommandBuffer::End:
				return;

			default:
				{
					BASE_ASSERT(CommandBuffer::RendererInit == command
						, "RendererInit must be the first command in command buffer before initialization. Unexpected command %d?"
						, command
						);
					BASE_ASSERT(!m_rendererInitialized, "This shouldn't happen! Bad synchronization?");

					Init init;
					_cmdbuf.read(init);

					m_renderCtx = rendererCreate(init);

					m_rendererInitialized = NULL != m_renderCtx;

					if (!m_rendererInitialized)
					{
						_cmdbuf.read(command);
						BASE_ASSERT(CommandBuffer::End == command, "Unexpected command %d?"
							, command
							);
						return;
					}
				}
				break;
			}
		}

		do
		{
			uint8_t command;
			_cmdbuf.read(command);

			switch (command)
			{
			case CommandBuffer::RendererShutdownBegin:
				{
					BASE_ASSERT(m_rendererInitialized, "This shouldn't happen! Bad synchronization?");
					m_rendererInitialized = false;
				}
				break;

			case CommandBuffer::RendererShutdownEnd:
				{
					BASE_ASSERT(!m_rendererInitialized && !m_exit, "This shouldn't happen! Bad synchronization?");

					rendererDestroy(m_renderCtx);
					m_renderCtx = NULL;

					m_exit = true;
				}
				BASE_FALLTHROUGH;

			case CommandBuffer::End:
				end = true;
				break;

			case CommandBuffer::CreateIndexBuffer:
				{
					GRAPHICS_PROFILER_SCOPE("CreateIndexBuffer", 0xff2040ff);

					IndexBufferHandle handle;
					_cmdbuf.read(handle);

					const Memory* mem;
					_cmdbuf.read(mem);

					uint16_t flags;
					_cmdbuf.read(flags);

					m_renderCtx->createIndexBuffer(handle, mem, flags);

					release(mem);
				}
				break;

			case CommandBuffer::DestroyIndexBuffer:
				{
					GRAPHICS_PROFILER_SCOPE("DestroyIndexBuffer", 0xff2040ff);

					IndexBufferHandle handle;
					_cmdbuf.read(handle);

					m_renderCtx->destroyIndexBuffer(handle);
				}
				break;

			case CommandBuffer::CreateVertexLayout:
				{
					GRAPHICS_PROFILER_SCOPE("CreateVertexLayout", 0xff2040ff);

					VertexLayoutHandle handle;
					_cmdbuf.read(handle);

					VertexLayout layout;
					_cmdbuf.read(layout);

					m_renderCtx->createVertexLayout(handle, layout);
				}
				break;

			case CommandBuffer::DestroyVertexLayout:
				{
					GRAPHICS_PROFILER_SCOPE("DestroyVertexLayout", 0xff2040ff);

					VertexLayoutHandle handle;
					_cmdbuf.read(handle);

					m_renderCtx->destroyVertexLayout(handle);
				}
				break;

			case CommandBuffer::CreateVertexBuffer:
				{
					GRAPHICS_PROFILER_SCOPE("CreateVertexBuffer", 0xff2040ff);

					VertexBufferHandle handle;
					_cmdbuf.read(handle);

					const Memory* mem;
					_cmdbuf.read(mem);

					VertexLayoutHandle layoutHandle;
					_cmdbuf.read(layoutHandle);

					uint16_t flags;
					_cmdbuf.read(flags);

					m_renderCtx->createVertexBuffer(handle, mem, layoutHandle, flags);

					release(mem);
				}
				break;

			case CommandBuffer::DestroyVertexBuffer:
				{
					GRAPHICS_PROFILER_SCOPE("DestroyVertexBuffer", 0xff2040ff);

					VertexBufferHandle handle;
					_cmdbuf.read(handle);

					m_renderCtx->destroyVertexBuffer(handle);
				}
				break;

			case CommandBuffer::CreateDynamicIndexBuffer:
				{
					GRAPHICS_PROFILER_SCOPE("CreateDynamicIndexBuffer", 0xff2040ff);

					IndexBufferHandle handle;
					_cmdbuf.read(handle);

					uint32_t size;
					_cmdbuf.read(size);

					uint16_t flags;
					_cmdbuf.read(flags);

					m_renderCtx->createDynamicIndexBuffer(handle, size, flags);
				}
				break;

			case CommandBuffer::UpdateDynamicIndexBuffer:
				{
					GRAPHICS_PROFILER_SCOPE("UpdateDynamicIndexBuffer", 0xff2040ff);

					IndexBufferHandle handle;
					_cmdbuf.read(handle);

					uint32_t offset;
					_cmdbuf.read(offset);

					uint32_t size;
					_cmdbuf.read(size);

					const Memory* mem;
					_cmdbuf.read(mem);

					m_renderCtx->updateDynamicIndexBuffer(handle, offset, size, mem);

					release(mem);
				}
				break;

			case CommandBuffer::DestroyDynamicIndexBuffer:
				{
					GRAPHICS_PROFILER_SCOPE("DestroyDynamicIndexBuffer", 0xff2040ff);

					IndexBufferHandle handle;
					_cmdbuf.read(handle);

					m_renderCtx->destroyDynamicIndexBuffer(handle);
				}
				break;

			case CommandBuffer::CreateDynamicVertexBuffer:
				{
					GRAPHICS_PROFILER_SCOPE("CreateDynamicVertexBuffer", 0xff2040ff);

					VertexBufferHandle handle;
					_cmdbuf.read(handle);

					uint32_t size;
					_cmdbuf.read(size);

					uint16_t flags;
					_cmdbuf.read(flags);

					m_renderCtx->createDynamicVertexBuffer(handle, size, flags);
				}
				break;

			case CommandBuffer::UpdateDynamicVertexBuffer:
				{
					GRAPHICS_PROFILER_SCOPE("UpdateDynamicVertexBuffer", 0xff2040ff);

					VertexBufferHandle handle;
					_cmdbuf.read(handle);

					uint32_t offset;
					_cmdbuf.read(offset);

					uint32_t size;
					_cmdbuf.read(size);

					const Memory* mem;
					_cmdbuf.read(mem);

					m_renderCtx->updateDynamicVertexBuffer(handle, offset, size, mem);

					release(mem);
				}
				break;

			case CommandBuffer::DestroyDynamicVertexBuffer:
				{
					GRAPHICS_PROFILER_SCOPE("DestroyDynamicVertexBuffer", 0xff2040ff);

					VertexBufferHandle handle;
					_cmdbuf.read(handle);

					m_renderCtx->destroyDynamicVertexBuffer(handle);
				}
				break;

			case CommandBuffer::CreateShader:
				{
					GRAPHICS_PROFILER_SCOPE("CreateShader", 0xff2040ff);

					ShaderHandle handle;
					_cmdbuf.read(handle);

					const Memory* mem;
					_cmdbuf.read(mem);

					m_renderCtx->createShader(handle, mem);

					release(mem);
				}
				break;

			case CommandBuffer::DestroyShader:
				{
					GRAPHICS_PROFILER_SCOPE("DestroyShader", 0xff2040ff);

					ShaderHandle handle;
					_cmdbuf.read(handle);

					m_renderCtx->destroyShader(handle);
				}
				break;

			case CommandBuffer::CreateProgram:
				{
					GRAPHICS_PROFILER_SCOPE("CreateProgram", 0xff2040ff);

					ProgramHandle handle;
					_cmdbuf.read(handle);

					ShaderHandle vsh;
					_cmdbuf.read(vsh);

					ShaderHandle fsh;
					_cmdbuf.read(fsh);

					m_renderCtx->createProgram(handle, vsh, fsh);
				}
				break;

			case CommandBuffer::DestroyProgram:
				{
					GRAPHICS_PROFILER_SCOPE("DestroyProgram", 0xff2040ff);

					ProgramHandle handle;
					_cmdbuf.read(handle);

					m_renderCtx->destroyProgram(handle);
				}
				break;

			case CommandBuffer::CreateTexture:
				{
					GRAPHICS_PROFILER_SCOPE("CreateTexture", 0xff2040ff);

					TextureHandle handle;
					_cmdbuf.read(handle);

					const Memory* mem;
					_cmdbuf.read(mem);

					uint64_t flags;
					_cmdbuf.read(flags);

					uint8_t skip;
					_cmdbuf.read(skip);

					void* ptr = m_renderCtx->createTexture(handle, mem, flags, skip);
					if (NULL != ptr)
					{
						setDirectAccessPtr(handle, ptr);
					}

					base::MemoryReader reader(mem->data, mem->size);
					base::Error err;

					uint32_t magic;
					base::read(&reader, magic, &err);

					if (GRAPHICS_CHUNK_MAGIC_TEX == magic)
					{
						TextureCreate tc;
						base::read(&reader, tc, &err);

						if (NULL != tc.m_mem)
						{
							release(tc.m_mem);
						}
					}

					release(mem);
				}
				break;

			case CommandBuffer::UpdateTexture:
				{
					GRAPHICS_PROFILER_SCOPE("UpdateTexture", 0xff2040ff);

					if (m_textureUpdateBatch.isFull() )
					{
						flushTextureUpdateBatch(_cmdbuf);
					}

					uint32_t value = _cmdbuf.m_pos;

					TextureHandle handle;
					_cmdbuf.read(handle);

					uint8_t side;
					_cmdbuf.read(side);

					uint8_t mip;
					_cmdbuf.read(mip);

					_cmdbuf.skip<Rect>();
					_cmdbuf.skip<uint16_t>();
					_cmdbuf.skip<uint16_t>();
					_cmdbuf.skip<uint16_t>();
					_cmdbuf.skip<Memory*>();

					uint32_t key = (handle.idx<<16)
						| (side<<8)
						| mip
						;

					m_textureUpdateBatch.add(key, value);
				}
				break;

			case CommandBuffer::ReadTexture:
				{
					GRAPHICS_PROFILER_SCOPE("ReadTexture", 0xff2040ff);

					TextureHandle handle;
					_cmdbuf.read(handle);

					void* data;
					_cmdbuf.read(data);

					uint8_t mip;
					_cmdbuf.read(mip);

					m_renderCtx->readTexture(handle, data, mip);
				}
				break;

			case CommandBuffer::ResizeTexture:
				{
					GRAPHICS_PROFILER_SCOPE("ResizeTexture", 0xff2040ff);

					TextureHandle handle;
					_cmdbuf.read(handle);

					uint16_t width;
					_cmdbuf.read(width);

					uint16_t height;
					_cmdbuf.read(height);

					uint8_t numMips;
					_cmdbuf.read(numMips);

					uint16_t numLayers;
					_cmdbuf.read(numLayers);

					m_renderCtx->resizeTexture(handle, width, height, numMips, numLayers);
				}
				break;

			case CommandBuffer::DestroyTexture:
				{
					GRAPHICS_PROFILER_SCOPE("DestroyTexture", 0xff2040ff);

					TextureHandle handle;
					_cmdbuf.read(handle);

					m_renderCtx->destroyTexture(handle);
				}
				break;

			case CommandBuffer::CreateFrameBuffer:
				{
					GRAPHICS_PROFILER_SCOPE("CreateFrameBuffer", 0xff2040ff);

					FrameBufferHandle handle;
					_cmdbuf.read(handle);

					bool window;
					_cmdbuf.read(window);

					if (window)
					{
						void* nwh;
						_cmdbuf.read(nwh);

						uint16_t width;
						_cmdbuf.read(width);

						uint16_t height;
						_cmdbuf.read(height);

						TextureFormat::Enum format;
						_cmdbuf.read(format);

						TextureFormat::Enum depthFormat;
						_cmdbuf.read(depthFormat);

						m_renderCtx->createFrameBuffer(handle, nwh, width, height, format, depthFormat);
					}
					else
					{
						uint8_t num;
						_cmdbuf.read(num);

						Attachment attachment[GRAPHICS_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS];
						_cmdbuf.read(attachment, sizeof(Attachment) * num);

						m_renderCtx->createFrameBuffer(handle, num, attachment);
					}
				}
				break;

			case CommandBuffer::DestroyFrameBuffer:
				{
					GRAPHICS_PROFILER_SCOPE("DestroyFrameBuffer", 0xff2040ff);

					FrameBufferHandle handle;
					_cmdbuf.read(handle);

					m_renderCtx->destroyFrameBuffer(handle);
				}
				break;

			case CommandBuffer::CreateUniform:
				{
					GRAPHICS_PROFILER_SCOPE("CreateUniform", 0xff2040ff);

					UniformHandle handle;
					_cmdbuf.read(handle);

					UniformType::Enum type;
					_cmdbuf.read(type);

					uint16_t num;
					_cmdbuf.read(num);

					uint8_t len;
					_cmdbuf.read(len);

					const char* name = (const char*)_cmdbuf.skip(len);

					m_renderCtx->createUniform(handle, type, num, name);
				}
				break;

			case CommandBuffer::DestroyUniform:
				{
					GRAPHICS_PROFILER_SCOPE("DestroyUniform", 0xff2040ff);

					UniformHandle handle;
					_cmdbuf.read(handle);

					m_renderCtx->destroyUniform(handle);
				}
				break;

			case CommandBuffer::UpdateViewName:
				{
					GRAPHICS_PROFILER_SCOPE("UpdateViewName", 0xff2040ff);

					ViewId id;
					_cmdbuf.read(id);

					uint16_t len;
					_cmdbuf.read(len);

					const char* name = (const char*)_cmdbuf.skip(len);

					m_renderCtx->updateViewName(id, name);
				}
				break;

			case CommandBuffer::InvalidateOcclusionQuery:
				{
					GRAPHICS_PROFILER_SCOPE("InvalidateOcclusionQuery", 0xff2040ff);

					OcclusionQueryHandle handle;
					_cmdbuf.read(handle);

					m_renderCtx->invalidateOcclusionQuery(handle);
				}
				break;

			case CommandBuffer::SetName:
				{
					GRAPHICS_PROFILER_SCOPE("SetName", 0xff2040ff);

					Handle handle;
					_cmdbuf.read(handle);

					uint16_t len;
					_cmdbuf.read(len);

					const char* name = (const char*)_cmdbuf.skip(len);

					m_renderCtx->setName(handle, name, len-1);
				}
				break;

			default:
				BASE_ASSERT(false, "Invalid command: %d", command);
				break;
			}
		} while (!end);

		flushTextureUpdateBatch(_cmdbuf);
	}

	uint32_t weldVertices(void* _output, const VertexLayout& _layout, const void* _data, uint32_t _num, bool _index32, float _epsilon)
	{
		return weldVertices(_output, _layout, _data, _num, _index32, _epsilon, g_allocator);
	}

	uint32_t topologyConvert(TopologyConvert::Enum _conversion, void* _dst, uint32_t _dstSize, const void* _indices, uint32_t _numIndices, bool _index32)
	{
		return topologyConvert(_conversion, _dst, _dstSize, _indices, _numIndices, _index32, g_allocator);
	}

	void topologySortTriList(TopologySort::Enum _sort, void* _dst, uint32_t _dstSize, const float _dir[3], const float _pos[3], const void* _vertices, uint32_t _stride, const void* _indices, uint32_t _numIndices, bool _index32)
	{
		topologySortTriList(_sort, _dst, _dstSize, _dir, _pos, _vertices, _stride, _indices, _numIndices, _index32, g_allocator);
	}

	uint8_t getSupportedRenderers(uint8_t _max, RendererType::Enum* _enum)
	{
		_enum = _max == 0 ? NULL : _enum;

		uint8_t num = 0;
		for (uint8_t ii = 0; ii < RendererType::Count; ++ii)
		{
			if ( (RendererType::Direct3D11 == ii || RendererType::Direct3D12 == ii)
			&&  windowsVersionIs(Condition::LessEqual, 0x0502) )
			{
				continue;
			}

			if (NULL == _enum)
			{
				num++;
			}
			else
			{
				if (num < _max
				&&  s_rendererCreator[ii].supported)
				{
					_enum[num++] = RendererType::Enum(ii);
				}
			}
		}

		return num;
	}

	const char* getRendererName(RendererType::Enum _type)
	{
		BASE_ASSERT(_type < RendererType::Count, "Invalid renderer type %d.", _type);
		return s_rendererCreator[_type].name;
	}

	PlatformData::PlatformData()
		: ndt(NULL)
		, nwh(NULL)
		, context(NULL)
		, backBuffer(NULL)
		, backBufferDS(NULL)
	{
	}

	Resolution::Resolution()
		: format(TextureFormat::RGBA8)
		, width(1280)
		, height(720)
		, reset(GRAPHICS_RESET_NONE)
		, numBackBuffers(2)
		, maxFrameLatency(0)
		, debugTextScale(0)
	{
	}

	Init::Limits::Limits()
		: maxEncoders(GRAPHICS_CONFIG_DEFAULT_MAX_ENCODERS)
		, minResourceCbSize(GRAPHICS_CONFIG_MIN_RESOURCE_COMMAND_BUFFER_SIZE)
		, transientVbSize(GRAPHICS_CONFIG_TRANSIENT_VERTEX_BUFFER_SIZE)
		, transientIbSize(GRAPHICS_CONFIG_TRANSIENT_INDEX_BUFFER_SIZE)
	{
	}

	Init::Init()
		: type(RendererType::Count)
		, vendorId(GRAPHICS_PCI_ID_NONE)
		, deviceId(0)
		, capabilities(UINT64_MAX)
		, debug(BASE_ENABLED(GRAPHICS_CONFIG_DEBUG) )
		, profile(BASE_ENABLED(GRAPHICS_CONFIG_DEBUG_ANNOTATION) )
		, callback(NULL)
		, allocator(NULL)
	{
	}

	void Attachment::init(TextureHandle _handle, Access::Enum _access, uint16_t _layer, uint16_t _numLayers, uint16_t _mip, uint8_t _resolve)
	{
		access    = _access;
		handle    = _handle;
		mip       = _mip;
		layer     = _layer;
		numLayers = _numLayers;
		resolve   = _resolve;
	}

	bool init(const Init& _userInit)
	{
		if (NULL != s_ctx)
		{
			BASE_TRACE("graphics is already initialized.");
			return false;
		}

		Init init = _userInit;

		init.limits.maxEncoders       = base::clamp<uint16_t>(init.limits.maxEncoders, 1, (0 != GRAPHICS_CONFIG_MULTITHREADED) ? 128 : 1);
		init.limits.minResourceCbSize = base::min<uint32_t>(init.limits.minResourceCbSize, GRAPHICS_CONFIG_MIN_RESOURCE_COMMAND_BUFFER_SIZE);

		struct ErrorState
		{
			enum Enum
			{
				Default,
				ContextAllocated,
			};
		};

		ErrorState::Enum errorState = ErrorState::Default;

		if (NULL != init.allocator)
		{
			g_allocator = init.allocator;
		}
		else
		{
			base::DefaultAllocator allocator;
			g_allocator =
				s_allocatorStub = BASE_NEW(&allocator, AllocatorStub);
		}

		if (NULL != init.callback)
		{
			g_callback = init.callback;
		}
		else
		{
			g_callback =
				s_callbackStub = BASE_NEW(g_allocator, CallbackStub);
		}

		base::memSet(&g_caps, 0, sizeof(g_caps) );
		g_caps.limits.maxDrawCalls            = GRAPHICS_CONFIG_MAX_DRAW_CALLS;
		g_caps.limits.maxBlits                = GRAPHICS_CONFIG_MAX_BLIT_ITEMS;
		g_caps.limits.maxTextureSize          = 0;
		g_caps.limits.maxTextureLayers        = 1;
		g_caps.limits.maxViews                = GRAPHICS_CONFIG_MAX_VIEWS;
		g_caps.limits.maxFrameBuffers         = GRAPHICS_CONFIG_MAX_FRAME_BUFFERS;
		g_caps.limits.maxPrograms             = GRAPHICS_CONFIG_MAX_PROGRAMS;
		g_caps.limits.maxShaders              = GRAPHICS_CONFIG_MAX_SHADERS;
		g_caps.limits.maxTextures             = GRAPHICS_CONFIG_MAX_TEXTURES;
		g_caps.limits.maxTextureSamplers      = GRAPHICS_CONFIG_MAX_TEXTURE_SAMPLERS;
		g_caps.limits.maxComputeBindings      = 0;
		g_caps.limits.maxVertexLayouts        = GRAPHICS_CONFIG_MAX_VERTEX_LAYOUTS;
		g_caps.limits.maxVertexStreams        = 1;
		g_caps.limits.maxIndexBuffers         = GRAPHICS_CONFIG_MAX_INDEX_BUFFERS;
		g_caps.limits.maxVertexBuffers        = GRAPHICS_CONFIG_MAX_VERTEX_BUFFERS;
		g_caps.limits.maxDynamicIndexBuffers  = GRAPHICS_CONFIG_MAX_DYNAMIC_INDEX_BUFFERS;
		g_caps.limits.maxDynamicVertexBuffers = GRAPHICS_CONFIG_MAX_DYNAMIC_VERTEX_BUFFERS;
		g_caps.limits.maxUniforms             = GRAPHICS_CONFIG_MAX_UNIFORMS;
		g_caps.limits.maxOcclusionQueries     = GRAPHICS_CONFIG_MAX_OCCLUSION_QUERIES;
		g_caps.limits.maxFBAttachments        = 1;
		g_caps.limits.maxEncoders             = init.limits.maxEncoders;
		g_caps.limits.minResourceCbSize       = init.limits.minResourceCbSize;
		g_caps.limits.transientVbSize         = init.limits.transientVbSize;
		g_caps.limits.transientIbSize         = init.limits.transientIbSize;

		g_caps.vendorId = init.vendorId;
		g_caps.deviceId = init.deviceId;

		BASE_TRACE("Init...");

		// graphics 1.104.7082
		//      ^ ^^^ ^^^^
		//      | |   +--- Commit number  (https://github.com/bkaradzic/graphics / git rev-list --count HEAD)
		//      | +------- API version    (from https://github.com/bkaradzic/graphics/blob/master/scripts/graphics.idl#L4)
		//      +--------- Major revision (always 1)
		BASE_TRACE("Version 1.%d.%d (commit: " GRAPHICS_REV_SHA1 ")", GRAPHICS_API_VERSION, GRAPHICS_REV_NUMBER);

		errorState = ErrorState::ContextAllocated;

		s_ctx = BASE_ALIGNED_NEW(g_allocator, Context, Context::kAlignment);
		if (s_ctx->init(init) )
		{
			BASE_TRACE("Init complete.");
			return true;
		}

		BASE_TRACE("Init failed.");

		switch (errorState)
		{
		case ErrorState::ContextAllocated:
			base::deleteObject(g_allocator, s_ctx, Context::kAlignment);
			s_ctx = NULL;
			BASE_FALLTHROUGH;

		case ErrorState::Default:
			if (NULL != s_callbackStub)
			{
				base::deleteObject(g_allocator, s_callbackStub);
				s_callbackStub = NULL;
			}

			if (NULL != s_allocatorStub)
			{
				base::DefaultAllocator allocator;
				base::deleteObject(&allocator, s_allocatorStub);
				s_allocatorStub = NULL;
			}

			s_threadIndex = 0;
			g_callback    = NULL;
			g_allocator   = NULL;
			break;
		}

		return false;
	}

	void shutdown()
	{
		BASE_TRACE("Shutdown...");

		GRAPHICS_CHECK_API_THREAD();
		Context* ctx = s_ctx; // it's going to be NULLd inside shutdown.
		ctx->shutdown();
		BASE_ASSERT(NULL == s_ctx, "graphics is should be uninitialized here.");

		base::deleteObject(g_allocator, ctx, Context::kAlignment);

		BASE_TRACE("Shutdown complete.");

		if (NULL != s_allocatorStub)
		{
			s_allocatorStub->checkLeaks();
		}

		if (NULL != s_callbackStub)
		{
			base::deleteObject(g_allocator, s_callbackStub);
			s_callbackStub = NULL;
		}

		if (NULL != s_allocatorStub)
		{
			base::DefaultAllocator allocator;
			base::deleteObject(&allocator, s_allocatorStub);
			s_allocatorStub = NULL;
		}

		s_threadIndex = 0;
		g_callback    = NULL;
		g_allocator   = NULL;
	}

	void reset(uint32_t _width, uint32_t _height, uint32_t _flags, TextureFormat::Enum _format)
	{
		GRAPHICS_CHECK_API_THREAD();
		BASE_ASSERT(0 == (_flags&GRAPHICS_RESET_RESERVED_MASK), "Do not set reset reserved flags!");
		s_ctx->reset(_width, _height, _flags, _format);
	}

	Encoder* begin(bool _forThread)
	{
		return s_ctx->begin(_forThread);
	}

#define GRAPHICS_ENCODER(_func) reinterpret_cast<EncoderImpl*>(this)->_func

	void Encoder::setMarker(const char* _marker)
	{
		GRAPHICS_ENCODER(setMarker(_marker) );
	}

	void Encoder::setState(uint64_t _state, uint32_t _rgba)
	{
		BASE_ASSERT(0 == (_state&GRAPHICS_STATE_RESERVED_MASK), "Do not set state reserved flags!");
		GRAPHICS_ENCODER(setState(_state, _rgba) );
	}

	void Encoder::setCondition(OcclusionQueryHandle _handle, bool _visible)
	{
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_OCCLUSION_QUERY, "Occlusion query is not supported!");
		GRAPHICS_ENCODER(setCondition(_handle, _visible) );
	}

	void Encoder::setStencil(uint32_t _fstencil, uint32_t _bstencil)
	{
		GRAPHICS_ENCODER(setStencil(_fstencil, _bstencil) );
	}

	uint16_t Encoder::setScissor(uint16_t _x, uint16_t _y, uint16_t _width, uint16_t _height)
	{
		return GRAPHICS_ENCODER(setScissor(_x, _y, _width, _height) );
	}

	void Encoder::setScissor(uint16_t _cache)
	{
		GRAPHICS_ENCODER(setScissor(_cache) );
	}

	uint32_t Encoder::setTransform(const void* _mtx, uint16_t _num)
	{
		return GRAPHICS_ENCODER(setTransform(_mtx, _num) );
	}

	uint32_t Encoder::allocTransform(Transform* _transform, uint16_t _num)
	{
		return GRAPHICS_ENCODER(allocTransform(_transform, _num) );
	}

	void Encoder::setTransform(uint32_t _cache, uint16_t _num)
	{
		GRAPHICS_ENCODER(setTransform(_cache, _num) );
	}

	void Encoder::setUniform(UniformHandle _handle, const void* _value, uint16_t _num)
	{
		GRAPHICS_CHECK_HANDLE("setUniform", s_ctx->m_uniformHandle, _handle);
		const UniformRef& uniform = s_ctx->m_uniformRef[_handle.idx];
		BASE_ASSERT(isValid(_handle) && 0 < uniform.m_refCount, "Setting invalid uniform (handle %3d)!", _handle.idx);
		BASE_ASSERT(_num == UINT16_MAX || uniform.m_num >= _num, "Truncated uniform update. %d (max: %d)", _num, uniform.m_num);
		GRAPHICS_ENCODER(setUniform(uniform.m_type, _handle, _value, UINT16_MAX != _num ? _num : uniform.m_num) );
	}

	void Encoder::setIndexBuffer(IndexBufferHandle _handle)
	{
		setIndexBuffer(_handle, 0, UINT32_MAX);
	}

	void Encoder::setIndexBuffer(IndexBufferHandle _handle, uint32_t _firstIndex, uint32_t _numIndices)
	{
		GRAPHICS_CHECK_HANDLE("setIndexBuffer", s_ctx->m_indexBufferHandle, _handle);
		const IndexBuffer& ib = s_ctx->m_indexBuffers[_handle.idx];
		GRAPHICS_ENCODER(setIndexBuffer(_handle, ib, _firstIndex, _numIndices) );
	}

	void Encoder::setIndexBuffer(DynamicIndexBufferHandle _handle)
	{
		setIndexBuffer(_handle, 0, UINT32_MAX);
	}

	void Encoder::setIndexBuffer(DynamicIndexBufferHandle _handle, uint32_t _firstIndex, uint32_t _numIndices)
	{
		GRAPHICS_CHECK_HANDLE("setIndexBuffer", s_ctx->m_dynamicIndexBufferHandle, _handle);
		const DynamicIndexBuffer& dib = s_ctx->m_dynamicIndexBuffers[_handle.idx];
		GRAPHICS_ENCODER(setIndexBuffer(dib, _firstIndex, _numIndices) );
	}

	void Encoder::setIndexBuffer(const TransientIndexBuffer* _tib)
	{
		setIndexBuffer(_tib, 0, UINT32_MAX);
	}

	void Encoder::setIndexBuffer(const TransientIndexBuffer* _tib, uint32_t _firstIndex, uint32_t _numIndices)
	{
		BASE_ASSERT(NULL != _tib, "_tib can't be NULL");
		GRAPHICS_CHECK_HANDLE("setIndexBuffer", s_ctx->m_indexBufferHandle, _tib->handle);
		GRAPHICS_ENCODER(setIndexBuffer(_tib, _firstIndex, _numIndices) );
	}

	void Encoder::setVertexBuffer(
		  uint8_t _stream
		, VertexBufferHandle _handle
		, uint32_t _startVertex
		, uint32_t _numVertices
		, VertexLayoutHandle _layoutHandle
	)
	{
		GRAPHICS_CHECK_HANDLE("setVertexBuffer", s_ctx->m_vertexBufferHandle, _handle);
		GRAPHICS_CHECK_HANDLE_INVALID_OK("setVertexBuffer", s_ctx->m_layoutHandle, _layoutHandle);
		GRAPHICS_ENCODER(setVertexBuffer(_stream, _handle, _startVertex, _numVertices, _layoutHandle) );
	}

	void Encoder::setVertexBuffer(uint8_t _stream, VertexBufferHandle _handle)
	{
		setVertexBuffer(_stream, _handle, 0, UINT32_MAX);
	}

	void Encoder::setVertexBuffer(
		  uint8_t _stream
		, DynamicVertexBufferHandle _handle
		, uint32_t _startVertex
		, uint32_t _numVertices
		, VertexLayoutHandle _layoutHandle
		)
	{
		GRAPHICS_CHECK_HANDLE("setVertexBuffer", s_ctx->m_dynamicVertexBufferHandle, _handle);
		GRAPHICS_CHECK_HANDLE_INVALID_OK("setVertexBuffer", s_ctx->m_layoutHandle, _layoutHandle);
		const DynamicVertexBuffer& dvb = s_ctx->m_dynamicVertexBuffers[_handle.idx];
		GRAPHICS_ENCODER(setVertexBuffer(_stream, dvb, _startVertex, _numVertices, _layoutHandle) );
	}

	void Encoder::setVertexBuffer(uint8_t _stream, DynamicVertexBufferHandle _handle)
	{
		setVertexBuffer(_stream, _handle, 0, UINT32_MAX);
	}

	void Encoder::setVertexBuffer(
		  uint8_t _stream
		, const TransientVertexBuffer* _tvb
		, uint32_t _startVertex
		, uint32_t _numVertices
		, VertexLayoutHandle _layoutHandle
		)
	{
		BASE_ASSERT(NULL != _tvb, "_tvb can't be NULL");
		GRAPHICS_CHECK_HANDLE("setVertexBuffer", s_ctx->m_vertexBufferHandle, _tvb->handle);
		GRAPHICS_CHECK_HANDLE_INVALID_OK("setVertexBuffer", s_ctx->m_layoutHandle, _layoutHandle);
		GRAPHICS_ENCODER(setVertexBuffer(_stream, _tvb, _startVertex, _numVertices, _layoutHandle) );
	}

	void Encoder::setVertexBuffer(uint8_t _stream, const TransientVertexBuffer* _tvb)
	{
		setVertexBuffer(_stream, _tvb, 0, UINT32_MAX);
	}

	void Encoder::setVertexCount(uint32_t _numVertices)
	{
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_VERTEX_ID, "Auto generated vertices are not supported!");
		GRAPHICS_ENCODER(setVertexCount(_numVertices) );
	}

	void Encoder::setInstanceDataBuffer(const InstanceDataBuffer* _idb)
	{
		setInstanceDataBuffer(_idb, 0, UINT32_MAX);
	}

	void Encoder::setInstanceDataBuffer(const InstanceDataBuffer* _idb, uint32_t _start, uint32_t _num)
	{
		BASE_ASSERT(NULL != _idb, "_idb can't be NULL");
		GRAPHICS_ENCODER(setInstanceDataBuffer(_idb, _start, _num) );
	}

	void Encoder::setInstanceDataBuffer(VertexBufferHandle _handle, uint32_t _startVertex, uint32_t _num)
	{
		GRAPHICS_CHECK_HANDLE("setInstanceDataBuffer", s_ctx->m_vertexBufferHandle, _handle);
		const VertexBuffer& vb = s_ctx->m_vertexBuffers[_handle.idx];
		GRAPHICS_ENCODER(setInstanceDataBuffer(_handle, _startVertex, _num, vb.m_stride) );
	}

	void Encoder::setInstanceDataBuffer(DynamicVertexBufferHandle _handle, uint32_t _startVertex, uint32_t _num)
	{
		GRAPHICS_CHECK_HANDLE("setInstanceDataBuffer", s_ctx->m_dynamicVertexBufferHandle, _handle);
		const DynamicVertexBuffer& dvb = s_ctx->m_dynamicVertexBuffers[_handle.idx];
		GRAPHICS_ENCODER(setInstanceDataBuffer(dvb.m_handle
			, dvb.m_startVertex + _startVertex
			, _num
			, dvb.m_stride
			) );
	}

	void Encoder::setInstanceCount(uint32_t _numInstances)
	{
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_VERTEX_ID, "Auto generated instances are not supported!");
		GRAPHICS_ENCODER(setInstanceCount(_numInstances) );
	}

	void Encoder::setTexture(uint8_t _stage, UniformHandle _sampler, TextureHandle _handle, uint32_t _flags)
	{
		GRAPHICS_CHECK_HANDLE("setTexture/UniformHandle", s_ctx->m_uniformHandle, _sampler);
		GRAPHICS_CHECK_HANDLE_INVALID_OK("setTexture/TextureHandle", s_ctx->m_textureHandle, _handle);
		BASE_ASSERT(_stage < g_caps.limits.maxTextureSamplers, "Invalid stage %d (max %d).", _stage, g_caps.limits.maxTextureSamplers);

		if (isValid(_handle) )
		{
			const TextureRef& ref = s_ctx->m_textureRef[_handle.idx];
			BASE_ASSERT(!ref.isReadBack()
				, "Can't sample from texture which was created with GRAPHICS_TEXTURE_READ_BACK. This is CPU only texture."
				);
			BASE_UNUSED(ref);
		}

		GRAPHICS_ENCODER(setTexture(_stage, _sampler, _handle, _flags) );
	}

	void Encoder::touch(ViewId _id)
	{
		discard();
		submit(_id, GRAPHICS_INVALID_HANDLE);
	}

	void Encoder::submit(ViewId _id, ProgramHandle _program, uint32_t _depth, uint8_t _flags)
	{
		OcclusionQueryHandle handle = GRAPHICS_INVALID_HANDLE;
		submit(_id, _program, handle, _depth, _flags);
	}

	void Encoder::submit(ViewId _id, ProgramHandle _program, OcclusionQueryHandle _occlusionQuery, uint32_t _depth, uint8_t _flags)
	{
		BASE_ASSERT(false
			|| !isValid(_occlusionQuery)
			|| 0 != (g_caps.supported & GRAPHICS_CAPS_OCCLUSION_QUERY)
			, "Occlusion query is not supported! Use graphics::getCaps to check GRAPHICS_CAPS_OCCLUSION_QUERY backend renderer capabilities."
			);
		GRAPHICS_CHECK_HANDLE_INVALID_OK("submit", s_ctx->m_programHandle, _program);
		GRAPHICS_CHECK_HANDLE_INVALID_OK("submit", s_ctx->m_occlusionQueryHandle, _occlusionQuery);
		GRAPHICS_ENCODER(submit(_id, _program, _occlusionQuery, _depth, _flags) );
	}

	void Encoder::submit(ViewId _id, ProgramHandle _program, IndirectBufferHandle _indirectHandle, uint16_t _start, uint16_t _num, uint32_t _depth, uint8_t _flags)
	{
		GRAPHICS_CHECK_HANDLE_INVALID_OK("submit", s_ctx->m_programHandle, _program);
		GRAPHICS_CHECK_HANDLE("submit", s_ctx->m_vertexBufferHandle, _indirectHandle);
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_DRAW_INDIRECT, "Draw indirect is not supported!");
		GRAPHICS_ENCODER(submit(_id, _program, _indirectHandle, _start, _num, _depth, _flags) );
	}

	void Encoder::submit(ViewId _id, ProgramHandle _program, IndirectBufferHandle _indirectHandle, uint16_t _start, IndexBufferHandle _numHandle, uint32_t _numIndex, uint16_t _numMax, uint32_t _depth, uint8_t _flags)
	{
		GRAPHICS_CHECK_HANDLE_INVALID_OK("submit", s_ctx->m_programHandle, _program);
		GRAPHICS_CHECK_HANDLE("submit", s_ctx->m_vertexBufferHandle, _indirectHandle);
		GRAPHICS_CHECK_HANDLE("submit", s_ctx->m_indexBufferHandle, _numHandle);
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_DRAW_INDIRECT, "Draw indirect is not supported!");
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_DRAW_INDIRECT_COUNT, "Draw indirect count is not supported!");
		GRAPHICS_ENCODER(submit(_id, _program, _indirectHandle, _start, _numHandle, _numIndex, _numMax, _depth, _flags) );
	}

	void Encoder::setBuffer(uint8_t _stage, IndexBufferHandle _handle, Access::Enum _access)
	{
		BASE_ASSERT(_stage < g_caps.limits.maxComputeBindings, "Invalid stage %d (max %d).", _stage, g_caps.limits.maxComputeBindings);
		GRAPHICS_CHECK_HANDLE("setBuffer", s_ctx->m_indexBufferHandle, _handle);
		GRAPHICS_ENCODER(setBuffer(_stage, _handle, _access) );
	}

	void Encoder::setBuffer(uint8_t _stage, VertexBufferHandle _handle, Access::Enum _access)
	{
		BASE_ASSERT(_stage < g_caps.limits.maxComputeBindings, "Invalid stage %d (max %d).", _stage, g_caps.limits.maxComputeBindings);
		GRAPHICS_CHECK_HANDLE("setBuffer", s_ctx->m_vertexBufferHandle, _handle);
		GRAPHICS_ENCODER(setBuffer(_stage, _handle, _access) );
	}

	void Encoder::setBuffer(uint8_t _stage, DynamicIndexBufferHandle _handle, Access::Enum _access)
	{
		BASE_ASSERT(_stage < g_caps.limits.maxComputeBindings, "Invalid stage %d (max %d).", _stage, g_caps.limits.maxComputeBindings);
		GRAPHICS_CHECK_HANDLE("setBuffer", s_ctx->m_dynamicIndexBufferHandle, _handle);
		const DynamicIndexBuffer& dib = s_ctx->m_dynamicIndexBuffers[_handle.idx];
		GRAPHICS_ENCODER(setBuffer(_stage, dib.m_handle, _access) );
	}

	void Encoder::setBuffer(uint8_t _stage, DynamicVertexBufferHandle _handle, Access::Enum _access)
	{
		BASE_ASSERT(_stage < g_caps.limits.maxComputeBindings, "Invalid stage %d (max %d).", _stage, g_caps.limits.maxComputeBindings);
		GRAPHICS_CHECK_HANDLE("setBuffer", s_ctx->m_dynamicVertexBufferHandle, _handle);
		const DynamicVertexBuffer& dvb = s_ctx->m_dynamicVertexBuffers[_handle.idx];
		GRAPHICS_ENCODER(setBuffer(_stage, dvb.m_handle, _access) );
	}

	void Encoder::setBuffer(uint8_t _stage, IndirectBufferHandle _handle, Access::Enum _access)
	{
		BASE_ASSERT(_stage < g_caps.limits.maxComputeBindings, "Invalid stage %d (max %d).", _stage, g_caps.limits.maxComputeBindings);
		GRAPHICS_CHECK_HANDLE("setBuffer", s_ctx->m_vertexBufferHandle, _handle);
		VertexBufferHandle handle = { _handle.idx };
		GRAPHICS_ENCODER(setBuffer(_stage, handle, _access) );
	}

	void Encoder::setImage(uint8_t _stage, TextureHandle _handle, uint8_t _mip, Access::Enum _access, TextureFormat::Enum _format)
	{
		BASE_ASSERT(_stage < g_caps.limits.maxComputeBindings, "Invalid stage %d (max %d).", _stage, g_caps.limits.maxComputeBindings);
		GRAPHICS_CHECK_HANDLE_INVALID_OK("setImage/TextureHandle", s_ctx->m_textureHandle, _handle);
		_format = TextureFormat::Count == _format
			? TextureFormat::Enum(s_ctx->m_textureRef[_handle.idx].m_format)
			: _format
			;
		BASE_ASSERT(_format != TextureFormat::BGRA8
			, "Can't use TextureFormat::BGRA8 with compute, use TextureFormat::RGBA8 instead."
			);

		if (isValid(_handle) )
		{
			const TextureRef& ref = s_ctx->m_textureRef[_handle.idx];
			BASE_ASSERT(!ref.isReadBack()
				, "Can't texture (handle %d, '%S') which was created with GRAPHICS_TEXTURE_READ_BACK with compute. This is CPU only texture."
				, _handle.idx
				, &ref.m_name
				);
			BASE_UNUSED(ref);
		}

		GRAPHICS_ENCODER(setImage(_stage, _handle, _mip, _access, _format) );
	}

	void Encoder::dispatch(ViewId _id, ProgramHandle _program, uint32_t _numX, uint32_t _numY, uint32_t _numZ, uint8_t _flags)
	{
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_COMPUTE, "Compute is not supported!");
		GRAPHICS_CHECK_HANDLE_INVALID_OK("dispatch", s_ctx->m_programHandle, _program);
		GRAPHICS_ENCODER(dispatch(_id, _program, _numX, _numY, _numZ, _flags) );
	}

	void Encoder::dispatch(ViewId _id, ProgramHandle _program, IndirectBufferHandle _indirectHandle, uint16_t _start, uint16_t _num, uint8_t _flags)
	{
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_DRAW_INDIRECT, "Dispatch indirect is not supported!");
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_COMPUTE, "Compute is not supported!");
		GRAPHICS_CHECK_HANDLE_INVALID_OK("dispatch", s_ctx->m_programHandle, _program);
		GRAPHICS_CHECK_HANDLE("dispatch", s_ctx->m_vertexBufferHandle, _indirectHandle);
		GRAPHICS_ENCODER(dispatch(_id, _program, _indirectHandle, _start, _num, _flags) );
	}

	void Encoder::discard(uint8_t _flags)
	{
		GRAPHICS_ENCODER(discard(_flags) );
	}

	void Encoder::blit(ViewId _id, TextureHandle _dst, uint16_t _dstX, uint16_t _dstY, TextureHandle _src, uint16_t _srcX, uint16_t _srcY, uint16_t _width, uint16_t _height)
	{
		blit(_id, _dst, 0, _dstX, _dstY, 0, _src, 0, _srcX, _srcY, 0, _width, _height, 0);
	}

	void Encoder::blit(ViewId _id, TextureHandle _dst, uint8_t _dstMip, uint16_t _dstX, uint16_t _dstY, uint16_t _dstZ, TextureHandle _src, uint8_t _srcMip, uint16_t _srcX, uint16_t _srcY, uint16_t _srcZ, uint16_t _width, uint16_t _height, uint16_t _depth)
	{
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_TEXTURE_BLIT, "Texture blit is not supported!");
		GRAPHICS_CHECK_HANDLE("blit/src TextureHandle", s_ctx->m_textureHandle, _src);
		GRAPHICS_CHECK_HANDLE("blit/dst TextureHandle", s_ctx->m_textureHandle, _dst);

		const TextureRef& src = s_ctx->m_textureRef[_src.idx];
		const TextureRef& dst = s_ctx->m_textureRef[_dst.idx];

		BASE_ASSERT(dst.isBlitDst()
			, "Blit destination texture (handle %d, '%S') is not created with `GRAPHICS_TEXTURE_BLIT_DST` flag."
			, _dst.idx
			, &dst.m_name
			);

		BASE_ASSERT(src.m_format == dst.m_format
			, "Texture format must match (src %s, dst %s)."
			, bimg::getName(bimg::TextureFormat::Enum(src.m_format) )
			, bimg::getName(bimg::TextureFormat::Enum(dst.m_format) )
			);
		BASE_ASSERT(_srcMip < src.m_numMips, "Invalid blit src mip (%d > %d)", _srcMip, src.m_numMips - 1);
		BASE_ASSERT(_dstMip < dst.m_numMips, "Invalid blit dst mip (%d > %d)", _dstMip, dst.m_numMips - 1);

		uint32_t srcWidth  = base::max<uint32_t>(1, src.m_width  >> _srcMip);
		uint32_t srcHeight = base::max<uint32_t>(1, src.m_height >> _srcMip);
		uint32_t dstWidth  = base::max<uint32_t>(1, dst.m_width  >> _dstMip);
		uint32_t dstHeight = base::max<uint32_t>(1, dst.m_height >> _dstMip);

		uint32_t srcDepth  = src.isCubeMap() ? 6 : base::max<uint32_t>(1, src.m_depth >> _srcMip);
		uint32_t dstDepth  = dst.isCubeMap() ? 6 : base::max<uint32_t>(1, dst.m_depth >> _dstMip);

		BASE_ASSERT(_srcX < srcWidth && _srcY < srcHeight && _srcZ < srcDepth
			, "Blit src coordinates out of range (%d, %d, %d) >= (%d, %d, %d)"
			, _srcX, _srcY, _srcZ
			, srcWidth, srcHeight, srcDepth
			);
		BASE_ASSERT(_dstX < dstWidth && _dstY < dstHeight && _dstZ < dstDepth
			, "Blit dst coordinates out of range (%d, %d, %d) >= (%d, %d, %d)"
			, _dstX, _dstY, _dstZ
			, dstWidth, dstHeight, dstDepth
			);

		srcWidth  = base::min<uint32_t>(srcWidth,  _srcX + _width ) - _srcX;
		srcHeight = base::min<uint32_t>(srcHeight, _srcY + _height) - _srcY;
		srcDepth  = base::min<uint32_t>(srcDepth,  _srcZ + _depth ) - _srcZ;
		dstWidth  = base::min<uint32_t>(dstWidth,  _dstX + _width ) - _dstX;
		dstHeight = base::min<uint32_t>(dstHeight, _dstY + _height) - _dstY;
		dstDepth  = base::min<uint32_t>(dstDepth,  _dstZ + _depth ) - _dstZ;

		const uint16_t width  = uint16_t(base::min(srcWidth,  dstWidth ) );
		const uint16_t height = uint16_t(base::min(srcHeight, dstHeight) );
		const uint16_t depth  = uint16_t(base::min(srcDepth,  dstDepth ) );

		GRAPHICS_ENCODER(blit(_id, _dst, _dstMip, _dstX, _dstY, _dstZ, _src, _srcMip, _srcX, _srcY, _srcZ, width, height, depth) );
	}

#undef GRAPHICS_ENCODER

	void end(Encoder* _encoder)
	{
		s_ctx->end(_encoder);
	}

	uint32_t frame(bool _capture)
	{
		GRAPHICS_CHECK_API_THREAD();
		return s_ctx->frame(_capture);
	}

	const Caps* getCaps()
	{
		return &g_caps;
	}

	const Stats* getStats()
	{
		return s_ctx->getPerfStats();
	}

	RendererType::Enum getRendererType()
	{
		return g_caps.rendererType;
	}

	const Memory* alloc(uint32_t _size)
	{
		BASE_ASSERT(0 < _size, "Invalid memory operation. _size is 0.");
		Memory* mem = (Memory*)base::alloc(g_allocator, sizeof(Memory) + _size);
		mem->size = _size;
		mem->data = (uint8_t*)mem + sizeof(Memory);
		return mem;
	}

	const Memory* copy(const void* _data, uint32_t _size)
	{
		BASE_ASSERT(0 < _size, "Invalid memory operation. _size is 0.");
		const Memory* mem = alloc(_size);
		base::memCopy(mem->data, _data, _size);
		return mem;
	}

	struct MemoryRef
	{
		Memory mem;
		ReleaseFn releaseFn;
		void* userData;
	};

	const Memory* makeRef(const void* _data, uint32_t _size, base::AllocatorI* _customAlloc, ReleaseFn _releaseFn, void* _userData)
	{
		if (g_allocator)
		{
			MemoryRef* memRef = (MemoryRef*)base::alloc(g_allocator, sizeof(MemoryRef));
			memRef->mem.size = _size;
			memRef->mem.data = (uint8_t*)_data;
			memRef->releaseFn = _releaseFn;
			memRef->userData = _userData;
			return &memRef->mem;
		}
		else
		{
			MemoryRef* memRef = (MemoryRef*)base::alloc(_customAlloc, sizeof(MemoryRef));
			memRef->mem.size = _size;
			memRef->mem.data = (uint8_t*)_data;
			memRef->releaseFn = _releaseFn;
			memRef->userData = _userData;
			return &memRef->mem;
		}
		
	}

	bool isMemoryRef(const Memory* _mem)
	{
		return _mem->data != (uint8_t*)_mem + sizeof(Memory);
	}

	void release(const Memory* _mem)
	{
		BASE_ASSERT(NULL != _mem, "_mem can't be NULL");
		Memory* mem = const_cast<Memory*>(_mem);
		if (isMemoryRef(mem) )
		{
			MemoryRef* memRef = reinterpret_cast<MemoryRef*>(mem);
			if (NULL != memRef->releaseFn)
			{
				memRef->releaseFn(mem->data, memRef->userData);
			}
		}
		base::free(g_allocator, mem);
	}

	void setDebug(uint32_t _debug)
	{
		GRAPHICS_CHECK_API_THREAD();
		s_ctx->setDebug(_debug);
	}

	void dbgTextClear(uint8_t _attr, bool _small)
	{
		GRAPHICS_CHECK_API_THREAD();
		s_ctx->dbgTextClear(_attr, _small);
	}

	void dbgTextPrintfVargs(uint16_t _x, uint16_t _y, uint8_t _attr, const char* _format, va_list _argList)
	{
		s_ctx->dbgTextPrintfVargs(_x, _y, _attr, _format, _argList);
	}

	void dbgTextPrintf(uint16_t _x, uint16_t _y, uint8_t _attr, const char* _format, ...)
	{
		GRAPHICS_CHECK_API_THREAD();
		va_list argList;
		va_start(argList, _format);
		s_ctx->dbgTextPrintfVargs(_x, _y, _attr, _format, argList);
		va_end(argList);
	}

	void dbgTextImage(uint16_t _x, uint16_t _y, uint16_t _width, uint16_t _height, const void* _data, uint16_t _pitch)
	{
		GRAPHICS_CHECK_API_THREAD();
		s_ctx->dbgTextImage(_x, _y, _width, _height, _data, _pitch);
	}

	IndexBufferHandle createIndexBuffer(const Memory* _mem, uint16_t _flags)
	{
		BASE_ASSERT(
			  0 == (_flags & GRAPHICS_BUFFER_INDEX32) || 0 != (g_caps.supported & GRAPHICS_CAPS_INDEX32)
			, "32-bit indices are not supported. Use graphics::getCaps to check GRAPHICS_CAPS_INDEX32 backend renderer capabilities."
			);
		BASE_ASSERT(NULL != _mem, "_mem can't be NULL");
		return s_ctx->createIndexBuffer(_mem, _flags);
	}

	void setName(IndexBufferHandle _handle, const char* _name, int32_t _len)
	{
		s_ctx->setName(_handle, base::StringView(_name, _len) );
	}

	void destroy(IndexBufferHandle _handle)
	{
		s_ctx->destroyIndexBuffer(_handle);
	}

	VertexLayoutHandle createVertexLayout(const VertexLayout& _layout)
	{
		return s_ctx->createVertexLayout(_layout);
	}

	void destroy(VertexLayoutHandle _handle)
	{
		s_ctx->destroyVertexLayout(_handle);
	}

	VertexBufferHandle createVertexBuffer(const Memory* _mem, const VertexLayout& _layout, uint16_t _flags)
	{
		BASE_ASSERT(NULL != _mem, "_mem can't be NULL");
		BASE_ASSERT(isValid(_layout), "Invalid VertexLayout.");
		return s_ctx->createVertexBuffer(_mem, _layout, _flags);
	}

	void setName(VertexBufferHandle _handle, const char* _name, int32_t _len)
	{
		s_ctx->setName(_handle, base::StringView(_name, _len) );
	}

	void destroy(VertexBufferHandle _handle)
	{
		s_ctx->destroyVertexBuffer(_handle);
	}

	DynamicIndexBufferHandle createDynamicIndexBuffer(uint32_t _num, uint16_t _flags)
	{
		return s_ctx->createDynamicIndexBuffer(_num, _flags);
	}

	DynamicIndexBufferHandle createDynamicIndexBuffer(const Memory* _mem, uint16_t _flags)
	{
		BASE_ASSERT(
			  0 == (_flags & GRAPHICS_BUFFER_INDEX32) || 0 != (g_caps.supported & GRAPHICS_CAPS_INDEX32)
			, "32-bit indices are not supported. Use graphics::getCaps to check GRAPHICS_CAPS_INDEX32 backend renderer capabilities."
			);
		BASE_ASSERT(NULL != _mem, "_mem can't be NULL");
		return s_ctx->createDynamicIndexBuffer(_mem, _flags);
	}

	void update(DynamicIndexBufferHandle _handle, uint32_t _startIndex, const Memory* _mem)
	{
		BASE_ASSERT(NULL != _mem, "_mem can't be NULL");
		s_ctx->update(_handle, _startIndex, _mem);
	}

	void destroy(DynamicIndexBufferHandle _handle)
	{
		s_ctx->destroyDynamicIndexBuffer(_handle);
	}

	DynamicVertexBufferHandle createDynamicVertexBuffer(uint32_t _num, const VertexLayout& _layout, uint16_t _flags)
	{
		BASE_ASSERT(isValid(_layout), "Invalid VertexLayout.");
		return s_ctx->createDynamicVertexBuffer(_num, _layout, _flags);
	}

	DynamicVertexBufferHandle createDynamicVertexBuffer(const Memory* _mem, const VertexLayout& _layout, uint16_t _flags)
	{
		BASE_ASSERT(NULL != _mem, "_mem can't be NULL");
		BASE_ASSERT(isValid(_layout), "Invalid VertexLayout.");
		return s_ctx->createDynamicVertexBuffer(_mem, _layout, _flags);
	}

	void update(DynamicVertexBufferHandle _handle, uint32_t _startVertex, const Memory* _mem)
	{
		BASE_ASSERT(NULL != _mem, "_mem can't be NULL");
		s_ctx->update(_handle, _startVertex, _mem);
	}

	void destroy(DynamicVertexBufferHandle _handle)
	{
		s_ctx->destroyDynamicVertexBuffer(_handle);
	}

	uint32_t getAvailTransientIndexBuffer(uint32_t _num, bool _index32)
	{
		BASE_ASSERT(0 < _num, "Requesting 0 indices.");
		return s_ctx->getAvailTransientIndexBuffer(_num, _index32);
	}

	uint32_t getAvailTransientVertexBuffer(uint32_t _num, const VertexLayout& _layout)
	{
		BASE_ASSERT(0 < _num, "Requesting 0 vertices.");
		BASE_ASSERT(isValid(_layout), "Invalid VertexLayout.");
		return s_ctx->getAvailTransientVertexBuffer(_num, _layout.m_stride);
	}

	uint32_t getAvailInstanceDataBuffer(uint32_t _num, uint16_t _stride)
	{
		BASE_ASSERT(0 < _num, "Requesting 0 instances.");
		return s_ctx->getAvailTransientVertexBuffer(_num, _stride);
	}

	void allocTransientIndexBuffer(TransientIndexBuffer* _tib, uint32_t _num, bool _index32)
	{
		BASE_ASSERT(NULL != _tib, "_tib can't be NULL");
		BASE_ASSERT(0 < _num, "Requesting 0 indices.");
		BASE_ASSERT(
			  !_index32 || 0 != (g_caps.supported & GRAPHICS_CAPS_INDEX32)
			, "32-bit indices are not supported. Use graphics::getCaps to check GRAPHICS_CAPS_INDEX32 backend renderer capabilities."
			);

		s_ctx->allocTransientIndexBuffer(_tib, _num, _index32);

		const uint32_t indexSize = _tib->isIndex16 ? 2 : 4;
		BASE_ASSERT(_num == _tib->size/ indexSize
			, "Failed to allocate transient index buffer (requested %d, available %d). "
			  "Use graphics::getAvailTransient* functions to ensure availability."
			, _num
			, _tib->size/indexSize
			);
		BASE_UNUSED(indexSize);
	}

	void allocTransientVertexBuffer(TransientVertexBuffer* _tvb, uint32_t _num, const VertexLayout& _layout)
	{
		BASE_ASSERT(NULL != _tvb, "_tvb can't be NULL");
		BASE_ASSERT(0 < _num, "Requesting 0 vertices.");
		BASE_ASSERT(isValid(_layout), "Invalid VertexLayout.");

		VertexLayoutHandle layoutHandle;
		{
			GRAPHICS_MUTEX_SCOPE(s_ctx->m_resourceApiLock);
			layoutHandle = s_ctx->findOrCreateVertexLayout(_layout, true);
		}
		BASE_ASSERT(isValid(layoutHandle), "Failed to allocate vertex layout handle (GRAPHICS_CONFIG_MAX_VERTEX_LAYOUTS, max: %d).", GRAPHICS_CONFIG_MAX_VERTEX_LAYOUTS);

		s_ctx->allocTransientVertexBuffer(_tvb, _num, layoutHandle, _layout.m_stride);

		BASE_ASSERT(_num == _tvb->size / _layout.m_stride
			, "Failed to allocate transient vertex buffer (requested %d, available %d). "
			  "Use graphics::getAvailTransient* functions to ensure availability."
			, _num
			, _tvb->size / _layout.m_stride
			);
	}

	bool allocTransientBuffers(graphics::TransientVertexBuffer* _tvb, const graphics::VertexLayout& _layout, uint32_t _numVertices, graphics::TransientIndexBuffer* _tib, uint32_t _numIndices, bool _index32)
	{
		GRAPHICS_MUTEX_SCOPE(s_ctx->m_resourceApiLock);

		if (_numVertices == getAvailTransientVertexBuffer(_numVertices, _layout)
		&&  _numIndices  == getAvailTransientIndexBuffer(_numIndices, _index32) )
		{
			allocTransientVertexBuffer(_tvb, _numVertices, _layout);
			allocTransientIndexBuffer(_tib, _numIndices, _index32);
			return true;
		}

		return false;
	}

	void allocInstanceDataBuffer(InstanceDataBuffer* _idb, uint32_t _num, uint16_t _stride)
	{
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_INSTANCING, "Instancing is not supported!");
		BASE_ASSERT(base::isAligned(_stride, 16), "Stride must be multiple of 16.");
		BASE_ASSERT(0 < _num, "Requesting 0 instanced data vertices.");
		s_ctx->allocInstanceDataBuffer(_idb, _num, _stride);
		BASE_ASSERT(_num == _idb->size / _stride
			, "Failed to allocate instance data buffer (requested %d, available %d). "
			  "Use graphics::getAvailTransient* functions to ensure availability."
			, _num
			, _idb->size / _stride
			);
	}

	IndirectBufferHandle createIndirectBuffer(uint32_t _num)
	{
		return s_ctx->createIndirectBuffer(_num);
	}

	void destroy(IndirectBufferHandle _handle)
	{
		s_ctx->destroyIndirectBuffer(_handle);
	}

	ShaderHandle createShader(const Memory* _mem)
	{
		BASE_ASSERT(NULL != _mem, "_mem can't be NULL");
		return s_ctx->createShader(_mem);
	}

	const graphics::Memory* compileShader(int _argc, const char* _argv[])
	{
		return s_ctx->compileShader(_argc, _argv);
	}

	uint16_t getShaderUniforms(ShaderHandle _handle, UniformHandle* _uniforms, uint16_t _max)
	{
		BASE_WARN(NULL == _uniforms || 0 != _max
			, "Passing uniforms array pointer, but array maximum capacity is set to 0."
			);

		uint16_t num = s_ctx->getShaderUniforms(_handle, _uniforms, _max);

		BASE_WARN(0 == _max || num <= _max
			, "Shader has more uniforms that capacity of output array. Output is truncated (num %d, max %d)."
			, num
			, _max
			);

		return num;
	}

	void setName(ShaderHandle _handle, const char* _name, int32_t _len)
	{
		s_ctx->setName(_handle, base::StringView(_name, _len) );
	}

	void destroy(ShaderHandle _handle)
	{
		s_ctx->destroyShader(_handle);
	}

	ProgramHandle createProgram(ShaderHandle _vsh, ShaderHandle _fsh, bool _destroyShaders)
	{
		if (!isValid(_fsh) )
		{
			return createProgram(_vsh, _destroyShaders);
		}

		return s_ctx->createProgram(_vsh, _fsh, _destroyShaders);
	}

	ProgramHandle createProgram(ShaderHandle _csh, bool _destroyShader)
	{
		return s_ctx->createProgram(_csh, _destroyShader);
	}

	void destroy(ProgramHandle _handle)
	{
		s_ctx->destroyProgram(_handle);
	}

	void isFrameBufferValid(uint8_t _num, const Attachment* _attachment, base::Error* _err)
	{
		BASE_ERROR_SCOPE(_err, "Frame buffer validation");

		uint8_t color = 0;
		uint8_t depth = 0;

		const TextureRef& firstTexture = s_ctx->m_textureRef[_attachment[0].handle.idx];

		const uint16_t firstAttachmentWidth  = base::max<uint16_t>(firstTexture.m_width  >> _attachment[0].mip, 1);
		const uint16_t firstAttachmentHeight = base::max<uint16_t>(firstTexture.m_height >> _attachment[0].mip, 1);

		for (uint32_t ii = 0; ii < _num; ++ii)
		{
			const Attachment&   at = _attachment[ii];
			const TextureHandle texHandle = at.handle;
			const TextureRef&   tr = s_ctx->m_textureRef[texHandle.idx];

			GRAPHICS_ERROR_CHECK(true
				&& isValid(texHandle)
				&& s_ctx->m_textureHandle.isValid(texHandle.idx)
				, _err
				, GRAPHICS_ERROR_FRAME_BUFFER_VALIDATION
				, "Invalid texture attachment."
				, "Attachment %d, texture handle %d."
				, ii
				, texHandle.idx
				);

			GRAPHICS_ERROR_CHECK(true
				&& at.mip < tr.m_numMips
				, _err
				, GRAPHICS_ERROR_FRAME_BUFFER_VALIDATION
				, "Invalid texture mip level."
				, "Attachment %d, Mip %d, texture (handle %d) number of mips %d."
				, ii
				, at.mip
				, texHandle.idx
				, tr.m_numMips
				);

			{
				const uint16_t numLayers = tr.is3D()
					? base::max<uint16_t>(tr.m_depth >> at.mip, 1)
					: tr.m_numLayers * (tr.isCubeMap() ? 6 : 1)
					;

				GRAPHICS_ERROR_CHECK(true
					&& (at.layer + at.numLayers) <= numLayers
					, _err
					, GRAPHICS_ERROR_FRAME_BUFFER_VALIDATION
					, "Invalid texture layer range."
					, "Attachment %d, Layer: %d, Num: %d, Max number of layers: %d."
					, ii
					, at.layer
					, at.numLayers
					, numLayers
					);
			}

			GRAPHICS_ERROR_CHECK(true
				&& _attachment[0].numLayers == at.numLayers
				, _err
				, GRAPHICS_ERROR_FRAME_BUFFER_VALIDATION
				, "Mismatch in attachment layer count."
				, "Attachment %d, Given: %d, Expected: %d."
				, ii
				, at.numLayers
				, _attachment[0].numLayers
				);

			GRAPHICS_ERROR_CHECK(true
				&& firstTexture.m_bbRatio == tr.m_bbRatio
				, _err
				, GRAPHICS_ERROR_FRAME_BUFFER_VALIDATION
				, "Mismatch in texture back-buffer ratio."
				, "Attachment %d, Given: %d, Expected: %d."
				, ii
				, tr.m_bbRatio
				, firstTexture.m_bbRatio
				);

			GRAPHICS_ERROR_CHECK(true
				&& firstTexture.m_numSamples == tr.m_numSamples
				, _err
				, GRAPHICS_ERROR_FRAME_BUFFER_VALIDATION
				, "Mismatch in texture sample count."
				, "Attachment %d, Given: %d, Expected: %d."
				, ii
				, tr.m_numSamples
				, firstTexture.m_numSamples
				);

			if (BackbufferRatio::Count == firstTexture.m_bbRatio)
			{
				const uint16_t width  = base::max<uint16_t>(tr.m_width  >> at.mip, 1);
				const uint16_t height = base::max<uint16_t>(tr.m_height >> at.mip, 1);

				GRAPHICS_ERROR_CHECK(true
					&& width  == firstAttachmentWidth
					&& height == firstAttachmentHeight
					, _err
					, GRAPHICS_ERROR_FRAME_BUFFER_VALIDATION
					, "Mismatch in texture size."
					, "Attachment %d, Given: %dx%d, Expected: %dx%d."
					, ii
					, width
					, height
					, firstAttachmentWidth
					, firstAttachmentHeight
					);
			}

			if (bimg::isDepth(bimg::TextureFormat::Enum(tr.m_format) ) )
			{
				++depth;

				GRAPHICS_ERROR_CHECK(
					// if GRAPHICS_TEXTURE_RT_MSAA_X2 or greater than GRAPHICS_TEXTURE_RT_WRITE_ONLY is required
					// if GRAPHICS_TEXTURE_RT with no MSSA then WRITE_ONLY is not required.
					(1 == ((tr.m_flags & GRAPHICS_TEXTURE_RT_MSAA_MASK) >> GRAPHICS_TEXTURE_RT_MSAA_SHIFT))
					|| (0 != (tr.m_flags & GRAPHICS_TEXTURE_RT_WRITE_ONLY))
					, _err
					, GRAPHICS_ERROR_FRAME_BUFFER_VALIDATION
					, "Frame buffer depth MSAA texture cannot be resolved. It must be created with `GRAPHICS_TEXTURE_RT_WRITE_ONLY` flag."
					, "Attachment %d, texture flags 0x%016" PRIx64 "."
					, ii
					, tr.m_flags
					);
			}
			else
			{
				++color;
			}

			GRAPHICS_ERROR_CHECK(true
				&& 0 == (tr.m_flags & GRAPHICS_TEXTURE_READ_BACK)
				, _err
				, GRAPHICS_ERROR_FRAME_BUFFER_VALIDATION
				, "Frame buffer texture cannot be created with `GRAPHICS_TEXTURE_READ_BACK`."
				, "Attachment %d, texture flags 0x%016" PRIx64 "."
				, ii
				, tr.m_flags
				);

			GRAPHICS_ERROR_CHECK(true
				&& 0 != (tr.m_flags & GRAPHICS_TEXTURE_RT_MASK)
				, _err
				, GRAPHICS_ERROR_FRAME_BUFFER_VALIDATION
				, "Frame buffer texture is not created with one of `GRAPHICS_TEXTURE_RT*` flags."
				, "Attachment %d, texture flags 0x%016" PRIx64 "."
				, ii
				, tr.m_flags
				);
		}

		GRAPHICS_ERROR_CHECK(true
			&& color <= g_caps.limits.maxFBAttachments
			, _err
			, GRAPHICS_ERROR_FRAME_BUFFER_VALIDATION
			, "Too many frame buffer color attachments."
			, "Num: %d, Max: %d."
			, _num
			, g_caps.limits.maxFBAttachments
			);

		GRAPHICS_ERROR_CHECK(true
			&& depth <= 1
			, _err
			, GRAPHICS_ERROR_FRAME_BUFFER_VALIDATION
			, "There can be only one depth texture attachment."
			, "Num depth attachments %d."
			, depth
			);
	}

	bool isFrameBufferValid(uint8_t _num, const Attachment* _attachment)
	{
		GRAPHICS_MUTEX_SCOPE(s_ctx->m_resourceApiLock);
		base::Error err;
		isFrameBufferValid(_num, _attachment, &err);
		return err.isOk();
	}

	static void isTextureValid(uint16_t _width, uint16_t _height, uint16_t _depth, bool _cubeMap, uint16_t _numLayers, TextureFormat::Enum _format, uint64_t _flags, base::Error* _err)
	{
		BASE_ERROR_SCOPE(_err, "Texture validation");

		const bool is3DTexture = 1 < _depth;

		GRAPHICS_ERROR_CHECK(false
			|| !_cubeMap
			|| !is3DTexture
			, _err
			, GRAPHICS_ERROR_TEXTURE_VALIDATION
			, "Texture can't be 3D and cube map at the same time."
			, ""
			);

		GRAPHICS_ERROR_CHECK(false
			|| !is3DTexture
			|| 0 != (g_caps.supported & GRAPHICS_CAPS_TEXTURE_3D)
			, _err
			, GRAPHICS_ERROR_TEXTURE_VALIDATION
			, "Texture3D is not supported! "
			  "Use graphics::getCaps to check `GRAPHICS_CAPS_TEXTURE_3D` backend renderer capabilities."
			, ""
			);

		GRAPHICS_ERROR_CHECK(false
			|| _width  <= g_caps.limits.maxTextureSize
			|| _height <= g_caps.limits.maxTextureSize
			, _err
			, GRAPHICS_ERROR_TEXTURE_VALIDATION
			, "Requested texture width/height is above the `maxTextureSize` limit."
			, "Texture width x height requested %d x %d (Max: %d)."
			, _width
			, _height
			, g_caps.limits.maxTextureSize
			);

		GRAPHICS_ERROR_CHECK(false
			|| 0 == (_flags & GRAPHICS_TEXTURE_RT_MASK)
			|| 0 == (_flags & GRAPHICS_TEXTURE_READ_BACK)
			, _err
			, GRAPHICS_ERROR_TEXTURE_VALIDATION
			, "Can't create render target with `GRAPHICS_TEXTURE_READ_BACK` flag."
			, ""
			);

		GRAPHICS_ERROR_CHECK(false
			|| 0 == (_flags & GRAPHICS_TEXTURE_COMPUTE_WRITE)
			|| 0 == (_flags & GRAPHICS_TEXTURE_READ_BACK)
			, _err
			, GRAPHICS_ERROR_TEXTURE_VALIDATION
			, "Can't create compute texture with `GRAPHICS_TEXTURE_READ_BACK` flag."
			, ""
			);

		GRAPHICS_ERROR_CHECK(false
			|| 1 >= _numLayers
			|| 0 != (g_caps.supported & GRAPHICS_CAPS_TEXTURE_2D_ARRAY)
			, _err
			, GRAPHICS_ERROR_TEXTURE_VALIDATION
			, "Texture array is not supported! "
			  "Use graphics::getCaps to check `GRAPHICS_CAPS_TEXTURE_2D_ARRAY` backend renderer capabilities."
			, ""
			);

		GRAPHICS_ERROR_CHECK(false
			|| _numLayers <= g_caps.limits.maxTextureLayers
			, _err
			, GRAPHICS_ERROR_TEXTURE_VALIDATION
			, "Requested number of texture array layers is above the `maxTextureLayers` limit."
			, "Number of texture array layers requested %d (Max: %d)."
			, _numLayers
			, g_caps.limits.maxTextureLayers
			);

		bool formatSupported;
		if (0 != (_flags & (GRAPHICS_TEXTURE_RT | GRAPHICS_TEXTURE_RT_WRITE_ONLY)) )
		{
			formatSupported = 0 != (g_caps.formats[_format] & GRAPHICS_CAPS_FORMAT_TEXTURE_FRAMEBUFFER);
		}
		else
		{
			formatSupported = 0 != (g_caps.formats[_format] & (0
				| GRAPHICS_CAPS_FORMAT_TEXTURE_2D
				| GRAPHICS_CAPS_FORMAT_TEXTURE_2D_EMULATED
				| GRAPHICS_CAPS_FORMAT_TEXTURE_2D_SRGB
				) );
		}

		uint16_t srgbCaps = GRAPHICS_CAPS_FORMAT_TEXTURE_2D_SRGB;

		if (_cubeMap)
		{
			formatSupported = 0 != (g_caps.formats[_format] & (0
				| GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE
				| GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE_EMULATED
				| GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE_SRGB
				) );
			srgbCaps = GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE_SRGB;
		}
		else if (is3DTexture)
		{
			formatSupported = 0 != (g_caps.formats[_format] & (0
				| GRAPHICS_CAPS_FORMAT_TEXTURE_3D
				| GRAPHICS_CAPS_FORMAT_TEXTURE_3D_EMULATED
				| GRAPHICS_CAPS_FORMAT_TEXTURE_3D_SRGB
				) );
			srgbCaps = GRAPHICS_CAPS_FORMAT_TEXTURE_3D_SRGB;
		}

		if (formatSupported
		&&  0 != (_flags & GRAPHICS_TEXTURE_RT_MASK) )
		{
			formatSupported = 0 != (g_caps.formats[_format] & (0
				| GRAPHICS_CAPS_FORMAT_TEXTURE_FRAMEBUFFER
				) );
		}

		GRAPHICS_ERROR_CHECK(
			  formatSupported
			, _err
			, GRAPHICS_ERROR_TEXTURE_VALIDATION
			, "Texture format is not supported! "
			  "Use graphics::isTextureValid to check support for texture format before creating it."
			, "Texture format: %s."
			, getName(_format)
			);

		GRAPHICS_ERROR_CHECK(false
			|| 0 == (_flags & GRAPHICS_TEXTURE_MSAA_SAMPLE)
			|| 0 != (g_caps.formats[_format] & GRAPHICS_CAPS_FORMAT_TEXTURE_MSAA)
			, _err
			, GRAPHICS_ERROR_TEXTURE_VALIDATION
			, "MSAA sampling for this texture format is not supported."
			, "Texture format: %s."
			, getName(_format)
			);

		GRAPHICS_ERROR_CHECK(false
			|| 0 == (_flags & GRAPHICS_TEXTURE_SRGB)
			|| 0 != (g_caps.formats[_format] & srgbCaps & (0
					| GRAPHICS_CAPS_FORMAT_TEXTURE_2D_SRGB
					| GRAPHICS_CAPS_FORMAT_TEXTURE_3D_SRGB
					| GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE_SRGB
					) )
			, _err
			, GRAPHICS_ERROR_TEXTURE_VALIDATION
			, "sRGB sampling for this texture format is not supported."
			, "Texture format: %s."
			, getName(_format)
			);
	}

	bool isTextureValid(uint16_t _depth, bool _cubeMap, uint16_t _numLayers, TextureFormat::Enum _format, uint64_t _flags)
	{
		base::Error err;
		isTextureValid(0, 0, _depth, _cubeMap, _numLayers, _format, _flags, &err);
		return err.isOk();
	}

	void isIdentifierValid(const base::StringView& _name, base::Error* _err)
	{
		BASE_ERROR_SCOPE(_err, "Uniform identifier validation");

		GRAPHICS_ERROR_CHECK(false
			|| !_name.isEmpty()
			, _err
			, GRAPHICS_ERROR_IDENTIFIER_VALIDATION
			, "Identifier can't be empty."
			, ""
			);

		GRAPHICS_ERROR_CHECK(false
			|| PredefinedUniform::Count == nameToPredefinedUniformEnum(_name)
			, _err
			, GRAPHICS_ERROR_IDENTIFIER_VALIDATION
			, "Identifier can't use predefined uniform name."
			, ""
			);

		const char ch = *_name.getPtr();
		GRAPHICS_ERROR_CHECK(false
			|| base::isAlpha(ch)
			|| '_' == ch
			, _err
			, GRAPHICS_ERROR_IDENTIFIER_VALIDATION
			, "The first character of an identifier should be either an alphabet character or an underscore."
			, ""
			);

		bool result = true;

		for (const char* ptr = _name.getPtr() + 1, *term = _name.getTerm()
			; ptr != term && result
			; ++ptr
			)
		{
			result &= base::isAlphaNum(*ptr) || '_' == *ptr;
		}

		GRAPHICS_ERROR_CHECK(false
			|| result
			, _err
			, GRAPHICS_ERROR_IDENTIFIER_VALIDATION
			, "Identifier contains invalid characters. Identifier must be the alphabet character, number, or underscore."
			, ""
			);
	}

	void calcTextureSize(TextureInfo& _info, uint16_t _width, uint16_t _height, uint16_t _depth, bool _cubeMap, bool _hasMips, uint16_t _numLayers, TextureFormat::Enum _format)
	{
		bimg::imageGetSize( (bimg::TextureInfo*)&_info, _width, _height, _depth, _cubeMap, _hasMips, _numLayers, bimg::TextureFormat::Enum(_format) );
	}

	TextureHandle createTexture(const Memory* _mem, uint64_t _flags, uint8_t _skip, TextureInfo* _info)
	{
		BASE_ASSERT(NULL != _mem, "_mem can't be NULL");
		return s_ctx->createTexture(_mem, _flags, _skip, _info, BackbufferRatio::Count, false);
	}

	void getTextureSizeFromRatio(BackbufferRatio::Enum _ratio, uint16_t& _width, uint16_t& _height)
	{
		switch (_ratio)
		{
		case BackbufferRatio::Half:      _width /=  2; _height /=  2; break;
		case BackbufferRatio::Quarter:   _width /=  4; _height /=  4; break;
		case BackbufferRatio::Eighth:    _width /=  8; _height /=  8; break;
		case BackbufferRatio::Sixteenth: _width /= 16; _height /= 16; break;
		case BackbufferRatio::Double:    _width *=  2; _height *=  2; break;

		default:
			break;
		}

		_width  = base::max<uint16_t>(1, _width);
		_height = base::max<uint16_t>(1, _height);
	}

	static TextureHandle createTexture2D(BackbufferRatio::Enum _ratio, uint16_t _width, uint16_t _height, bool _hasMips, uint16_t _numLayers, TextureFormat::Enum _format, uint64_t _flags, const Memory* _mem)
	{
		if (BackbufferRatio::Count != _ratio)
		{
			_width  = uint16_t(s_ctx->m_init.resolution.width);
			_height = uint16_t(s_ctx->m_init.resolution.height);
			getTextureSizeFromRatio(_ratio, _width, _height);
		}

		base::ErrorAssert err;
		isTextureValid(_width, _height, 0, false, _numLayers, _format, _flags, &err);

		if (!err.isOk() )
		{
			return GRAPHICS_INVALID_HANDLE;
		}

		const uint8_t numMips = calcNumMips(_hasMips, _width, _height);
		_numLayers = base::max<uint16_t>(_numLayers, 1);

		if (BASE_ENABLED(GRAPHICS_CONFIG_DEBUG)
		&&  NULL != _mem)
		{
			TextureInfo ti;
			calcTextureSize(ti, _width, _height, 1, false, _hasMips, _numLayers, _format);
			BASE_ASSERT(ti.storageSize == _mem->size
				, "createTexture2D: Texture storage size doesn't match passed memory size (storage size: %d, memory size: %d)"
				, ti.storageSize
				, _mem->size
				);
		}

		uint32_t size = sizeof(uint32_t)+sizeof(TextureCreate);
		const Memory* mem = alloc(size);

		base::StaticMemoryBlockWriter writer(mem->data, mem->size);
		uint32_t magic = GRAPHICS_CHUNK_MAGIC_TEX;
		base::write(&writer, magic, base::ErrorAssert{});

		TextureCreate tc;
		tc.m_width     = _width;
		tc.m_height    = _height;
		tc.m_depth     = 0;
		tc.m_numLayers = _numLayers;
		tc.m_numMips   = numMips;
		tc.m_format    = _format;
		tc.m_cubeMap   = false;
		tc.m_mem       = _mem;
		base::write(&writer, tc, base::ErrorAssert{});

		return s_ctx->createTexture(mem, _flags, 0, NULL, _ratio, NULL != _mem);
	}

	TextureHandle createTexture2D(uint16_t _width, uint16_t _height, bool _hasMips, uint16_t _numLayers, TextureFormat::Enum _format, uint64_t _flags, const Memory* _mem)
	{
		BASE_ASSERT(_width > 0 && _height > 0, "Invalid texture size (width %d, height %d).", _width, _height);
		return createTexture2D(BackbufferRatio::Count, _width, _height, _hasMips, _numLayers, _format, _flags, _mem);
	}

	TextureHandle createTexture2D(BackbufferRatio::Enum _ratio, bool _hasMips, uint16_t _numLayers, TextureFormat::Enum _format, uint64_t _flags)
	{
		BASE_ASSERT(_ratio < BackbufferRatio::Count, "Invalid back buffer ratio.");
		return createTexture2D(_ratio, 0, 0, _hasMips, _numLayers, _format, _flags, NULL);
	}

	TextureHandle createTexture3D(uint16_t _width, uint16_t _height, uint16_t _depth, bool _hasMips, TextureFormat::Enum _format, uint64_t _flags, const Memory* _mem)
	{
		base::ErrorAssert err;
		isTextureValid(_width, _height, _depth, false, 1, _format, _flags, &err);

		if (!err.isOk() )
		{
			return GRAPHICS_INVALID_HANDLE;
		}

		const uint8_t numMips = calcNumMips(_hasMips, _width, _height, _depth);

		if (BASE_ENABLED(GRAPHICS_CONFIG_DEBUG)
		&&  NULL != _mem)
		{
			TextureInfo ti;
			calcTextureSize(ti, _width, _height, _depth, false, _hasMips, 1, _format);
			BASE_ASSERT(ti.storageSize == _mem->size
				, "createTexture3D: Texture storage size doesn't match passed memory size (storage size: %d, memory size: %d)"
				, ti.storageSize
				, _mem->size
				);
		}

		uint32_t size = sizeof(uint32_t)+sizeof(TextureCreate);
		const Memory* mem = alloc(size);

		base::StaticMemoryBlockWriter writer(mem->data, mem->size);
		uint32_t magic = GRAPHICS_CHUNK_MAGIC_TEX;
		base::write(&writer, magic, base::ErrorAssert{});

		TextureCreate tc;
		tc.m_width     = _width;
		tc.m_height    = _height;
		tc.m_depth     = _depth;
		tc.m_numLayers = 1;
		tc.m_numMips   = numMips;
		tc.m_format    = _format;
		tc.m_cubeMap   = false;
		tc.m_mem       = _mem;
		base::write(&writer, tc, base::ErrorAssert{});

		return s_ctx->createTexture(mem, _flags, 0, NULL, BackbufferRatio::Count, NULL != _mem);
	}

	TextureHandle createTextureCube(uint16_t _size, bool _hasMips, uint16_t _numLayers, TextureFormat::Enum _format, uint64_t _flags, const Memory* _mem)
	{
		base::ErrorAssert err;
		isTextureValid(_size, _size, 0, true, _numLayers, _format, _flags, &err);

		if (!err.isOk() )
		{
			return GRAPHICS_INVALID_HANDLE;
		}

		const uint8_t numMips = calcNumMips(_hasMips, _size, _size);
		_numLayers = base::max<uint16_t>(_numLayers, 1);

		if (BASE_ENABLED(GRAPHICS_CONFIG_DEBUG)
		&&  NULL != _mem)
		{
			TextureInfo ti;
			calcTextureSize(ti, _size, _size, 1, true, _hasMips, _numLayers, _format);
			BASE_ASSERT(ti.storageSize == _mem->size
				, "createTextureCube: Texture storage size doesn't match passed memory size (storage size: %d, memory size: %d)"
				, ti.storageSize
				, _mem->size
				);
		}

		uint32_t size = sizeof(uint32_t)+sizeof(TextureCreate);
		const Memory* mem = alloc(size);

		base::StaticMemoryBlockWriter writer(mem->data, mem->size);
		uint32_t magic = GRAPHICS_CHUNK_MAGIC_TEX;
		base::write(&writer, magic, base::ErrorAssert{});

		TextureCreate tc;
		tc.m_width     = _size;
		tc.m_height    = _size;
		tc.m_depth     = 0;
		tc.m_numLayers = _numLayers;
		tc.m_numMips   = numMips;
		tc.m_format    = _format;
		tc.m_cubeMap   = true;
		tc.m_mem       = _mem;
		base::write(&writer, tc, base::ErrorAssert{});

		return s_ctx->createTexture(mem, _flags, 0, NULL, BackbufferRatio::Count, NULL != _mem);
	}

	void setName(TextureHandle _handle, const char* _name, int32_t _len)
	{
		s_ctx->setName(_handle, base::StringView(_name, _len) );
	}

	void* getDirectAccessPtr(TextureHandle _handle)
	{
		return s_ctx->getDirectAccessPtr(_handle);
	}

	void destroy(TextureHandle _handle)
	{
		s_ctx->destroyTexture(_handle);
	}

	void updateTexture2D(TextureHandle _handle, uint16_t _layer, uint8_t _mip, uint16_t _x, uint16_t _y, uint16_t _width, uint16_t _height, const Memory* _mem, uint16_t _pitch)
	{
		BASE_ASSERT(NULL != _mem, "_mem can't be NULL");
		if (_width  == 0
		||  _height == 0)
		{
			release(_mem);
		}
		else
		{
			s_ctx->updateTexture(_handle, 0, _mip, _x, _y, _layer, _width, _height, 1, _pitch, _mem);
		}
	}

	void updateTexture3D(TextureHandle _handle, uint8_t _mip, uint16_t _x, uint16_t _y, uint16_t _z, uint16_t _width, uint16_t _height, uint16_t _depth, const Memory* _mem)
	{
		BASE_ASSERT(NULL != _mem, "_mem can't be NULL");
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_TEXTURE_3D, "Texture3D is not supported!");

		if (0 == _width
		||  0 == _height
		||  0 == _depth)
		{
			release(_mem);
		}
		else
		{
			s_ctx->updateTexture(_handle, 0, _mip, _x, _y, _z, _width, _height, _depth, UINT16_MAX, _mem);
		}
	}

	void updateTextureCube(TextureHandle _handle, uint16_t _layer, uint8_t _side, uint8_t _mip, uint16_t _x, uint16_t _y, uint16_t _width, uint16_t _height, const Memory* _mem, uint16_t _pitch)
	{
		BASE_ASSERT(NULL != _mem, "_mem can't be NULL");
		BASE_ASSERT(_side <= 5, "Invalid side %d.", _side);
		if (0 == _width
		||  0 == _height)
		{
			release(_mem);
		}
		else
		{
			s_ctx->updateTexture(_handle, _side, _mip, _x, _y, _layer, _width, _height, 1, _pitch, _mem);
		}
	}

	uint32_t readTexture(TextureHandle _handle, void* _data, uint8_t _mip)
	{
		BASE_ASSERT(NULL != _data, "_data can't be NULL");
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_TEXTURE_READ_BACK, "Texture read-back is not supported!");
		return s_ctx->readTexture(_handle, _data, _mip);
	}

	FrameBufferHandle createFrameBuffer(uint16_t _width, uint16_t _height, TextureFormat::Enum _format, uint64_t _textureFlags)
	{
		_textureFlags |= _textureFlags&GRAPHICS_TEXTURE_RT_MSAA_MASK ? 0 : GRAPHICS_TEXTURE_RT;
		TextureHandle th = createTexture2D(_width, _height, false, 1, _format, _textureFlags);
		return createFrameBuffer(1, &th, true);
	}

	FrameBufferHandle createFrameBuffer(BackbufferRatio::Enum _ratio, TextureFormat::Enum _format, uint64_t _textureFlags)
	{
		BASE_ASSERT(_ratio < BackbufferRatio::Count, "Invalid back buffer ratio.");
		_textureFlags |= _textureFlags&GRAPHICS_TEXTURE_RT_MSAA_MASK ? 0 : GRAPHICS_TEXTURE_RT;
		TextureHandle th = createTexture2D(_ratio, false, 1, _format, _textureFlags);
		return createFrameBuffer(1, &th, true);
	}

	FrameBufferHandle createFrameBuffer(uint8_t _num, const TextureHandle* _handles, bool _destroyTextures)
	{
		Attachment attachment[GRAPHICS_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS];
		for (uint8_t ii = 0; ii < _num; ++ii)
		{
			Attachment& at = attachment[ii];
			at.init(_handles[ii], Access::Write, 0, 1, 0, GRAPHICS_RESOLVE_AUTO_GEN_MIPS);
		}
		return createFrameBuffer(_num, attachment, _destroyTextures);
	}

	FrameBufferHandle createFrameBuffer(uint8_t _num, const Attachment* _attachment, bool _destroyTextures)
	{
		BASE_ASSERT(_num != 0, "Number of frame buffer attachments can't be 0.");
		BASE_ASSERT(_num <= GRAPHICS_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS
			, "Number of frame buffer attachments is larger than allowed %d (max: %d)."
			, _num
			, GRAPHICS_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS
			);
		BASE_ASSERT(NULL != _attachment, "_attachment can't be NULL");
		return s_ctx->createFrameBuffer(_num, _attachment, _destroyTextures);
	}

	FrameBufferHandle createFrameBuffer(void* _nwh, uint16_t _width, uint16_t _height, TextureFormat::Enum _format, TextureFormat::Enum _depthFormat)
	{
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_SWAP_CHAIN, "Swap chain is not supported!");
		BASE_WARN(_width > 0 && _height > 0
			, "Invalid frame buffer dimensions (width %d, height %d)."
			, _width
			, _height
			);
		BASE_ASSERT(_format == TextureFormat::Count || bimg::isColor(bimg::TextureFormat::Enum(_format) )
			, "Invalid texture format for color (%s)."
			, bimg::getName(bimg::TextureFormat::Enum(_format) )
			);
		BASE_ASSERT(_depthFormat == TextureFormat::Count || bimg::isDepth(bimg::TextureFormat::Enum(_depthFormat) )
			, "Invalid texture format for depth (%s)."
			, bimg::getName(bimg::TextureFormat::Enum(_depthFormat) )
			);
		return s_ctx->createFrameBuffer(
			  _nwh
			, base::max<uint16_t>(_width, 1)
			, base::max<uint16_t>(_height, 1)
			, _format
			, _depthFormat
			);
	}

	void setName(FrameBufferHandle _handle, const char* _name, int32_t _len)
	{
		s_ctx->setName(_handle, base::StringView(_name, _len) );
	}

	TextureHandle getTexture(FrameBufferHandle _handle, uint8_t _attachment)
	{
		return s_ctx->getTexture(_handle, _attachment);
	}

	void destroy(FrameBufferHandle _handle)
	{
		s_ctx->destroyFrameBuffer(_handle);
	}

	UniformHandle createUniform(const char* _name, UniformType::Enum _type, uint16_t _num)
	{
		return s_ctx->createUniform(_name, _type, _num);
	}

	void getUniformInfo(UniformHandle _handle, UniformInfo& _info)
	{
		s_ctx->getUniformInfo(_handle, _info);
	}

	void destroy(UniformHandle _handle)
	{
		s_ctx->destroyUniform(_handle);
	}

	OcclusionQueryHandle createOcclusionQuery()
	{
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_OCCLUSION_QUERY, "Occlusion query is not supported!");
		return s_ctx->createOcclusionQuery();
	}

	OcclusionQueryResult::Enum getResult(OcclusionQueryHandle _handle, int32_t* _result)
	{
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_OCCLUSION_QUERY, "Occlusion query is not supported!");
		return s_ctx->getResult(_handle, _result);
	}

	void destroy(OcclusionQueryHandle _handle)
	{
		GRAPHICS_CHECK_CAPS(GRAPHICS_CAPS_OCCLUSION_QUERY, "Occlusion query is not supported!");
		s_ctx->destroyOcclusionQuery(_handle);
	}

	void setPaletteColor(uint8_t _index, uint32_t _rgba)
	{
		const uint8_t rr = uint8_t(_rgba>>24);
		const uint8_t gg = uint8_t(_rgba>>16);
		const uint8_t bb = uint8_t(_rgba>> 8);
		const uint8_t aa = uint8_t(_rgba>> 0);

		const float rgba[4] =
		{
			rr * 1.0f/255.0f,
			gg * 1.0f/255.0f,
			bb * 1.0f/255.0f,
			aa * 1.0f/255.0f,
		};

		s_ctx->setPaletteColor(_index, rgba);
	}

	void setPaletteColor(uint8_t _index, float _r, float _g, float _b, float _a)
	{
		float rgba[4] = { _r, _g, _b, _a };
		s_ctx->setPaletteColor(_index, rgba);
	}

	void setPaletteColor(uint8_t _index, const float _rgba[4])
	{
		s_ctx->setPaletteColor(_index, _rgba);
	}

	bool checkView(ViewId _id)
	{
		// workaround GCC 4.9 type-limit check.
		const uint32_t id = _id;
		return id < GRAPHICS_CONFIG_MAX_VIEWS;
	}

	void setViewName(ViewId _id, const char* _name)
	{
		BASE_ASSERT(checkView(_id), "Invalid view id: %d", _id);
		s_ctx->setViewName(_id, _name);
	}

	void setViewRect(ViewId _id, uint16_t _x, uint16_t _y, uint16_t _width, uint16_t _height)
	{
		BASE_ASSERT(checkView(_id), "Invalid view id: %d", _id);
		s_ctx->setViewRect(_id, _x, _y, _width, _height);
	}

	void setViewRect(ViewId _id, uint16_t _x, uint16_t _y, BackbufferRatio::Enum _ratio)
	{
		BASE_ASSERT(checkView(_id), "Invalid view id: %d", _id);

		uint16_t width  = uint16_t(s_ctx->m_init.resolution.width);
		uint16_t height = uint16_t(s_ctx->m_init.resolution.height);
		getTextureSizeFromRatio(_ratio, width, height);
		setViewRect(_id, _x, _y, width, height);
	}

	void setViewScissor(ViewId _id, uint16_t _x, uint16_t _y, uint16_t _width, uint16_t _height)
	{
		BASE_ASSERT(checkView(_id), "Invalid view id: %d", _id);
		s_ctx->setViewScissor(_id, _x, _y, _width, _height);
	}

	void setViewClear(ViewId _id, uint16_t _flags, uint32_t _rgba, float _depth, uint8_t _stencil)
	{
		BASE_ASSERT(checkView(_id), "Invalid view id: %d", _id);
		s_ctx->setViewClear(_id, _flags, _rgba, _depth, _stencil);
	}

	void setViewClear(ViewId _id, uint16_t _flags, float _depth, uint8_t _stencil, uint8_t _0, uint8_t _1, uint8_t _2, uint8_t _3, uint8_t _4, uint8_t _5, uint8_t _6, uint8_t _7)
	{
		BASE_ASSERT(checkView(_id), "Invalid view id: %d", _id);
		s_ctx->setViewClear(_id, _flags, _depth, _stencil, _0, _1, _2, _3, _4, _5, _6, _7);
	}

	void setViewMode(ViewId _id, ViewMode::Enum _mode)
	{
		BASE_ASSERT(checkView(_id), "Invalid view id: %d", _id);
		s_ctx->setViewMode(_id, _mode);
	}

	void setViewFrameBuffer(ViewId _id, FrameBufferHandle _handle)
	{
		BASE_ASSERT(checkView(_id), "Invalid view id: %d", _id);
		s_ctx->setViewFrameBuffer(_id, _handle);
	}

	void setViewTransform(ViewId _id, const void* _view, const void* _proj)
	{
		BASE_ASSERT(checkView(_id), "Invalid view id: %d", _id);
		s_ctx->setViewTransform(_id, _view, _proj);
	}

	void setViewOrder(ViewId _id, uint16_t _num, const ViewId* _order)
	{
		BASE_ASSERT(checkView(_id), "Invalid view id: %d", _id);
		s_ctx->setViewOrder(_id, _num, _order);
	}

	void resetView(ViewId _id)
	{
		BASE_ASSERT(checkView(_id), "Invalid view id: %d", _id);
		s_ctx->resetView(_id);
	}

#define GRAPHICS_CHECK_ENCODER0()                               \
	GRAPHICS_CHECK_API_THREAD();                                \
	GRAPHICS_FATAL(NULL != s_ctx->m_encoder0, Fatal::DebugCheck \
		, "graphics is configured to allow only encoder API. See: `GRAPHICS_CONFIG_ENCODER_API_ONLY`.")

	void setMarker(const char* _marker)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setMarker(_marker);
	}

	void setState(uint64_t _state, uint32_t _rgba)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setState(_state, _rgba);
	}

	void setCondition(OcclusionQueryHandle _handle, bool _visible)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setCondition(_handle, _visible);
	}

	void setStencil(uint32_t _fstencil, uint32_t _bstencil)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setStencil(_fstencil, _bstencil);
	}

	uint16_t setScissor(uint16_t _x, uint16_t _y, uint16_t _width, uint16_t _height)
	{
		GRAPHICS_CHECK_ENCODER0();
		return s_ctx->m_encoder0->setScissor(_x, _y, _width, _height);
	}

	void setScissor(uint16_t _cache)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setScissor(_cache);
	}

	uint32_t setTransform(const void* _mtx, uint16_t _num)
	{
		GRAPHICS_CHECK_ENCODER0();
		return s_ctx->m_encoder0->setTransform(_mtx, _num);
	}

	uint32_t allocTransform(Transform* _transform, uint16_t _num)
	{
		GRAPHICS_CHECK_ENCODER0();
		return s_ctx->m_encoder0->allocTransform(_transform, _num);
	}

	void setTransform(uint32_t _cache, uint16_t _num)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setTransform(_cache, _num);
	}

	void setUniform(UniformHandle _handle, const void* _value, uint16_t _num)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setUniform(_handle, _value, _num);
	}

	void setIndexBuffer(IndexBufferHandle _handle)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setIndexBuffer(_handle);
	}

	void setIndexBuffer(IndexBufferHandle _handle, uint32_t _firstIndex, uint32_t _numIndices)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setIndexBuffer(_handle, _firstIndex, _numIndices);
	}

	void setIndexBuffer(DynamicIndexBufferHandle _handle)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setIndexBuffer(_handle);
	}

	void setIndexBuffer(DynamicIndexBufferHandle _handle, uint32_t _firstIndex, uint32_t _numIndices)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setIndexBuffer(_handle, _firstIndex, _numIndices);
	}

	void setIndexBuffer(const TransientIndexBuffer* _tib)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setIndexBuffer(_tib);
	}

	void setIndexBuffer(const TransientIndexBuffer* _tib, uint32_t _firstIndex, uint32_t _numIndices)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setIndexBuffer(_tib, _firstIndex, _numIndices);
	}

	void setVertexBuffer(
		  uint8_t _stream
		, VertexBufferHandle _handle
		, uint32_t _startVertex
		, uint32_t _numVertices
		, VertexLayoutHandle _layoutHandle
		)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setVertexBuffer(_stream, _handle, _startVertex, _numVertices, _layoutHandle);
	}

	void setVertexBuffer(uint8_t _stream, VertexBufferHandle _handle)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setVertexBuffer(_stream, _handle);
	}

	void setVertexBuffer(
		  uint8_t _stream
		, DynamicVertexBufferHandle _handle
		, uint32_t _startVertex
		, uint32_t _numVertices
		, VertexLayoutHandle _layoutHandle
		)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setVertexBuffer(_stream, _handle, _startVertex, _numVertices, _layoutHandle);
	}

	void setVertexBuffer(uint8_t _stream, DynamicVertexBufferHandle _handle)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setVertexBuffer(_stream, _handle);
	}

	void setVertexBuffer(
		  uint8_t _stream
		, const TransientVertexBuffer* _tvb
		, uint32_t _startVertex
		, uint32_t _numVertices
		, VertexLayoutHandle _layoutHandle
		)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setVertexBuffer(_stream, _tvb, _startVertex, _numVertices, _layoutHandle);
	}

	void setVertexBuffer(uint8_t _stream, const TransientVertexBuffer* _tvb)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setVertexBuffer(_stream, _tvb);
	}

	void setVertexCount(uint32_t _numVertices)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setVertexCount(_numVertices);
	}

	void setInstanceDataBuffer(const InstanceDataBuffer* _idb)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setInstanceDataBuffer(_idb);
	}

	void setInstanceDataBuffer(const InstanceDataBuffer* _idb, uint32_t _start, uint32_t _num)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setInstanceDataBuffer(_idb, _start, _num);
	}

	void setInstanceDataBuffer(VertexBufferHandle _handle, uint32_t _startVertex, uint32_t _num)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setInstanceDataBuffer(_handle, _startVertex, _num);
	}

	void setInstanceDataBuffer(DynamicVertexBufferHandle _handle, uint32_t _startVertex, uint32_t _num)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setInstanceDataBuffer(_handle, _startVertex, _num);
	}

	void setInstanceCount(uint32_t _numInstances)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setInstanceCount(_numInstances);
	}

	void setTexture(uint8_t _stage, UniformHandle _sampler, TextureHandle _handle, uint32_t _flags)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setTexture(_stage, _sampler, _handle, _flags);
	}

	void touch(ViewId _id)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->touch(_id);
	}

	void submit(ViewId _id, ProgramHandle _program, uint32_t _depth, uint8_t _flags)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->submit(_id, _program, _depth, _flags);
	}

	void submit(ViewId _id, ProgramHandle _program, OcclusionQueryHandle _occlusionQuery, uint32_t _depth, uint8_t _flags)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->submit(_id, _program, _occlusionQuery, _depth, _flags);
	}

	void submit(ViewId _id, ProgramHandle _program, IndirectBufferHandle _indirectHandle, uint16_t _start, uint16_t _num, uint32_t _depth, uint8_t _flags)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->submit(_id, _program, _indirectHandle, _start, _num, _depth, _flags);
	}

	void submit(ViewId _id, ProgramHandle _program, IndirectBufferHandle _indirectHandle, uint16_t _start, IndexBufferHandle _numHandle, uint32_t _numIndex, uint16_t _numMax, uint32_t _depth, uint8_t _flags)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->submit(_id, _program, _indirectHandle, _start, _numHandle, _numIndex, _numMax, _depth, _flags);
	}

	void setBuffer(uint8_t _stage, IndexBufferHandle _handle, Access::Enum _access)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setBuffer(_stage, _handle, _access);
	}

	void setBuffer(uint8_t _stage, VertexBufferHandle _handle, Access::Enum _access)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setBuffer(_stage, _handle, _access);
	}

	void setBuffer(uint8_t _stage, DynamicIndexBufferHandle _handle, Access::Enum _access)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setBuffer(_stage, _handle, _access);
	}

	void setBuffer(uint8_t _stage, DynamicVertexBufferHandle _handle, Access::Enum _access)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setBuffer(_stage, _handle, _access);
	}

	void setBuffer(uint8_t _stage, IndirectBufferHandle _handle, Access::Enum _access)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setBuffer(_stage, _handle, _access);
	}

	void setImage(uint8_t _stage, TextureHandle _handle, uint8_t _mip, Access::Enum _access, TextureFormat::Enum _format)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->setImage(_stage, _handle, _mip, _access, _format);
	}

	void dispatch(ViewId _id, ProgramHandle _handle, uint32_t _numX, uint32_t _numY, uint32_t _numZ, uint8_t _flags)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->dispatch(_id, _handle, _numX, _numY, _numZ, _flags);
	}

	void dispatch(ViewId _id, ProgramHandle _handle, IndirectBufferHandle _indirectHandle, uint16_t _start, uint16_t _num, uint8_t _flags)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->dispatch(_id, _handle, _indirectHandle, _start, _num, _flags);
	}

	void discard(uint8_t _flags)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->discard(_flags);
	}

	void blit(ViewId _id, TextureHandle _dst, uint16_t _dstX, uint16_t _dstY, TextureHandle _src, uint16_t _srcX, uint16_t _srcY, uint16_t _width, uint16_t _height)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->blit(_id, _dst, _dstX, _dstY, _src, _srcX, _srcY, _width, _height);
	}

	void blit(ViewId _id, TextureHandle _dst, uint8_t _dstMip, uint16_t _dstX, uint16_t _dstY, uint16_t _dstZ, TextureHandle _src, uint8_t _srcMip, uint16_t _srcX, uint16_t _srcY, uint16_t _srcZ, uint16_t _width, uint16_t _height, uint16_t _depth)
	{
		GRAPHICS_CHECK_ENCODER0();
		s_ctx->m_encoder0->blit(_id, _dst, _dstMip, _dstX, _dstY, _dstZ, _src, _srcMip, _srcX, _srcY, _srcZ, _width, _height, _depth);
	}

	void requestScreenShot(FrameBufferHandle _handle, const char* _filePath)
	{
		GRAPHICS_CHECK_API_THREAD();
		s_ctx->requestScreenShot(_handle, _filePath);
	}

#undef GRAPHICS_CHECK_ENCODER0

} // namespace graphics

#if GRAPHICS_CONFIG_PREFER_DISCRETE_GPU
extern "C"
{
	// When laptop setup has integrated and discrete GPU, following driver workarounds will
	// select discrete GPU:

	// Reference(s):
	// - https://web.archive.org/web/20180722051003/https://docs.nvidia.com/gameworks/content/technologies/desktop/optimus.htm
	//
	__declspec(dllexport) uint32_t NvOptimusEnablement = UINT32_C(1);

	// Reference(s):
	// - https://web.archive.org/web/20180722051032/https://gpuopen.com/amdpowerxpressrequesthighperformance/
	//
	__declspec(dllexport) uint32_t AmdPowerXpressRequestHighPerformance = UINT32_C(1);
}
#endif // GRAPHICS_CONFIG_PREFER_DISCRETE_GPU

#define GRAPHICS_TEXTURE_FORMAT_BIMG(_fmt) \
	BASE_STATIC_ASSERT(uint32_t(graphics::TextureFormat::_fmt) == uint32_t(bimg::TextureFormat::_fmt) )

GRAPHICS_TEXTURE_FORMAT_BIMG(BC1);
GRAPHICS_TEXTURE_FORMAT_BIMG(BC2);
GRAPHICS_TEXTURE_FORMAT_BIMG(BC3);
GRAPHICS_TEXTURE_FORMAT_BIMG(BC4);
GRAPHICS_TEXTURE_FORMAT_BIMG(BC5);
GRAPHICS_TEXTURE_FORMAT_BIMG(BC6H);
GRAPHICS_TEXTURE_FORMAT_BIMG(BC7);
GRAPHICS_TEXTURE_FORMAT_BIMG(ETC1);
GRAPHICS_TEXTURE_FORMAT_BIMG(ETC2);
GRAPHICS_TEXTURE_FORMAT_BIMG(ETC2A);
GRAPHICS_TEXTURE_FORMAT_BIMG(ETC2A1);
GRAPHICS_TEXTURE_FORMAT_BIMG(PTC12);
GRAPHICS_TEXTURE_FORMAT_BIMG(PTC14);
GRAPHICS_TEXTURE_FORMAT_BIMG(PTC12A);
GRAPHICS_TEXTURE_FORMAT_BIMG(PTC14A);
GRAPHICS_TEXTURE_FORMAT_BIMG(PTC22);
GRAPHICS_TEXTURE_FORMAT_BIMG(PTC24);
GRAPHICS_TEXTURE_FORMAT_BIMG(ATC);
GRAPHICS_TEXTURE_FORMAT_BIMG(ATCE);
GRAPHICS_TEXTURE_FORMAT_BIMG(ATCI);
GRAPHICS_TEXTURE_FORMAT_BIMG(ASTC4x4);
GRAPHICS_TEXTURE_FORMAT_BIMG(ASTC5x4);
GRAPHICS_TEXTURE_FORMAT_BIMG(ASTC5x5);
GRAPHICS_TEXTURE_FORMAT_BIMG(ASTC6x5);
GRAPHICS_TEXTURE_FORMAT_BIMG(ASTC6x6);
GRAPHICS_TEXTURE_FORMAT_BIMG(ASTC8x5);
GRAPHICS_TEXTURE_FORMAT_BIMG(ASTC8x6);
GRAPHICS_TEXTURE_FORMAT_BIMG(ASTC8x8);
GRAPHICS_TEXTURE_FORMAT_BIMG(ASTC10x5);
GRAPHICS_TEXTURE_FORMAT_BIMG(ASTC10x6);
GRAPHICS_TEXTURE_FORMAT_BIMG(ASTC10x8);
GRAPHICS_TEXTURE_FORMAT_BIMG(ASTC10x10);
GRAPHICS_TEXTURE_FORMAT_BIMG(ASTC12x10);
GRAPHICS_TEXTURE_FORMAT_BIMG(ASTC12x12);
GRAPHICS_TEXTURE_FORMAT_BIMG(Unknown);
GRAPHICS_TEXTURE_FORMAT_BIMG(R1);
GRAPHICS_TEXTURE_FORMAT_BIMG(A8);
GRAPHICS_TEXTURE_FORMAT_BIMG(R8);
GRAPHICS_TEXTURE_FORMAT_BIMG(R8I);
GRAPHICS_TEXTURE_FORMAT_BIMG(R8U);
GRAPHICS_TEXTURE_FORMAT_BIMG(R8S);
GRAPHICS_TEXTURE_FORMAT_BIMG(R16);
GRAPHICS_TEXTURE_FORMAT_BIMG(R16I);
GRAPHICS_TEXTURE_FORMAT_BIMG(R16U);
GRAPHICS_TEXTURE_FORMAT_BIMG(R16F);
GRAPHICS_TEXTURE_FORMAT_BIMG(R16S);
GRAPHICS_TEXTURE_FORMAT_BIMG(R32I);
GRAPHICS_TEXTURE_FORMAT_BIMG(R32U);
GRAPHICS_TEXTURE_FORMAT_BIMG(R32F);
GRAPHICS_TEXTURE_FORMAT_BIMG(RG8);
GRAPHICS_TEXTURE_FORMAT_BIMG(RG8I);
GRAPHICS_TEXTURE_FORMAT_BIMG(RG8U);
GRAPHICS_TEXTURE_FORMAT_BIMG(RG8S);
GRAPHICS_TEXTURE_FORMAT_BIMG(RG16);
GRAPHICS_TEXTURE_FORMAT_BIMG(RG16I);
GRAPHICS_TEXTURE_FORMAT_BIMG(RG16U);
GRAPHICS_TEXTURE_FORMAT_BIMG(RG16F);
GRAPHICS_TEXTURE_FORMAT_BIMG(RG16S);
GRAPHICS_TEXTURE_FORMAT_BIMG(RG32I);
GRAPHICS_TEXTURE_FORMAT_BIMG(RG32U);
GRAPHICS_TEXTURE_FORMAT_BIMG(RG32F);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGB8);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGB8I);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGB8U);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGB8S);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGB9E5F);
GRAPHICS_TEXTURE_FORMAT_BIMG(BGRA8);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGBA8);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGBA8I);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGBA8U);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGBA8S);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGBA16);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGBA16I);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGBA16U);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGBA16F);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGBA16S);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGBA32I);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGBA32U);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGBA32F);
GRAPHICS_TEXTURE_FORMAT_BIMG(B5G6R5);
GRAPHICS_TEXTURE_FORMAT_BIMG(R5G6B5);
GRAPHICS_TEXTURE_FORMAT_BIMG(BGRA4);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGBA4);
GRAPHICS_TEXTURE_FORMAT_BIMG(BGR5A1);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGB5A1);
GRAPHICS_TEXTURE_FORMAT_BIMG(RGB10A2);
GRAPHICS_TEXTURE_FORMAT_BIMG(RG11B10F);
GRAPHICS_TEXTURE_FORMAT_BIMG(UnknownDepth);
GRAPHICS_TEXTURE_FORMAT_BIMG(D16);
GRAPHICS_TEXTURE_FORMAT_BIMG(D24);
GRAPHICS_TEXTURE_FORMAT_BIMG(D24S8);
GRAPHICS_TEXTURE_FORMAT_BIMG(D32);
GRAPHICS_TEXTURE_FORMAT_BIMG(D16F);
GRAPHICS_TEXTURE_FORMAT_BIMG(D24F);
GRAPHICS_TEXTURE_FORMAT_BIMG(D32F);
GRAPHICS_TEXTURE_FORMAT_BIMG(D0S8);
GRAPHICS_TEXTURE_FORMAT_BIMG(Count);

#undef GRAPHICS_TEXTURE_FORMAT_BIMG



//#include "graphics.idl.inl"


