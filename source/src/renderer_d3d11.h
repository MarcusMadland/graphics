/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#ifndef GRAPHICS_RENDERER_D3D11_H_HEADER_GUARD
#define GRAPHICS_RENDERER_D3D11_H_HEADER_GUARD

#define USE_D3D11_DYNAMIC_LIB    (BASE_PLATFORM_LINUX || BASE_PLATFORM_WINDOWS)
#define USE_D3D11_STAGING_BUFFER 0

#if !USE_D3D11_DYNAMIC_LIB
#   undef  GRAPHICS_CONFIG_DEBUG_ANNOTATION
#   define GRAPHICS_CONFIG_DEBUG_ANNOTATION 0
#endif // !USE_D3D11_DYNAMIC_LIB

BASE_PRAGMA_DIAGNOSTIC_PUSH();
BASE_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunknown-pragmas" );
BASE_PRAGMA_DIAGNOSTIC_IGNORED_GCC("-Wpragmas");
BASE_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4005) // warning C4005: '' : macro redefinition

#include <sal.h>
#include <unknwn.h>

#define D3D11_NO_HELPERS
#if BASE_PLATFORM_LINUX || BASE_PLATFORM_WINDOWS
#   include <d3d11_3.h>
#elif BASE_PLATFORM_WINRT
#   define __D3D10_1SHADER_H__ // BK - not used keep quiet!
#   include <d3d11_3.h>
#else
#   if !GRAPHICS_CONFIG_DEBUG
#      define D3DCOMPILE_NO_DEBUG_AND_ALL_FAST_SEMANTICS 1
#   endif // !GRAPHICS_CONFIG_DEBUG
#   include <d3d11_x.h>
#endif // BASE_PLATFORM_*
BASE_PRAGMA_DIAGNOSTIC_POP()

#include "renderer.h"
#include "renderer_d3d.h"
#include "shader_dxbc.h"
#include "debug_renderdoc.h"
#include "nvapi.h"
#include "dxgi.h"

#define GRAPHICS_D3D11_BLEND_STATE_MASK (0   \
	| GRAPHICS_STATE_BLEND_MASK              \
	| GRAPHICS_STATE_BLEND_EQUATION_MASK     \
	| GRAPHICS_STATE_BLEND_INDEPENDENT       \
	| GRAPHICS_STATE_BLEND_ALPHA_TO_COVERAGE \
	| GRAPHICS_STATE_WRITE_A                 \
	| GRAPHICS_STATE_WRITE_RGB               \
	)

#define GRAPHICS_D3D11_DEPTH_STENCIL_MASK (0 \
	| GRAPHICS_STATE_WRITE_Z                 \
	| GRAPHICS_STATE_DEPTH_TEST_MASK         \
	)

#define GRAPHICS_D3D11_PROFILER_BEGIN(_view, _abgr)         \
	BASE_MACRO_BLOCK_BEGIN                                \
		PIX_BEGINEVENT(_abgr, s_viewNameW[_view]);      \
		GRAPHICS_PROFILER_BEGIN(s_viewName[view], _abgr);   \
	BASE_MACRO_BLOCK_END

#define GRAPHICS_D3D11_PROFILER_BEGIN_LITERAL(_name, _abgr) \
	BASE_MACRO_BLOCK_BEGIN                                \
		PIX_BEGINEVENT(_abgr, L"" _name);               \
		GRAPHICS_PROFILER_BEGIN_LITERAL("" _name, _abgr);   \
	BASE_MACRO_BLOCK_END

#define GRAPHICS_D3D11_PROFILER_END()                       \
	BASE_MACRO_BLOCK_BEGIN                                \
		GRAPHICS_PROFILER_END();                            \
		PIX_ENDEVENT();                                 \
	BASE_MACRO_BLOCK_END

namespace graphics { namespace d3d11
{
	struct BufferD3D11
	{
		BufferD3D11()
			: m_ptr(NULL)
#if USE_D3D11_STAGING_BUFFER
			, m_staging(NULL)
#endif // USE_D3D11_STAGING_BUFFER
			, m_srv(NULL)
			, m_uav(NULL)
			, m_flags(GRAPHICS_BUFFER_NONE)
			, m_dynamic(false)
		{
		}

		void create(uint32_t _size, void* _data, uint16_t _flags, uint16_t _stride = 0, bool _vertex = false);
		void update(uint32_t _offset, uint32_t _size, void* _data, bool _discard = false);

		void destroy()
		{
			if (NULL != m_ptr)
			{
				DX_RELEASE(m_ptr, 0);
				m_dynamic = false;
			}

#if USE_D3D11_STAGING_BUFFER
			DX_RELEASE(m_staging, 0);
#endif // USE_D3D11_STAGING_BUFFER

			DX_RELEASE(m_srv, 0);
			DX_RELEASE(m_uav, 0);
		}

		ID3D11Buffer* m_ptr;
#if USE_D3D11_STAGING_BUFFER
		ID3D11Buffer* m_staging;
#endif // USE_D3D11_STAGING_BUFFER
		ID3D11ShaderResourceView*  m_srv;
		ID3D11UnorderedAccessView* m_uav;
		uint32_t m_size;
		uint16_t m_flags;
		bool m_dynamic;
	};

	typedef BufferD3D11 IndexBufferD3D11;

	struct VertexBufferD3D11 : public BufferD3D11
	{
		VertexBufferD3D11()
			: BufferD3D11()
		{
		}

		void create(uint32_t _size, void* _data, VertexLayoutHandle _layoutHandle, uint16_t _flags);

		VertexLayoutHandle m_layoutHandle;
	};

	struct ShaderD3D11
	{
		ShaderD3D11()
			: m_ptr(NULL)
			, m_code(NULL)
			, m_buffer(NULL)
			, m_constantBuffer(NULL)
			, m_hash(0)
			, m_numUniforms(0)
			, m_numPredefined(0)
			, m_hasDepthOp(false)
		{
		}

		void create(const Memory* _mem);

		void destroy()
		{
			if (NULL != m_constantBuffer)
			{
				UniformBuffer::destroy(m_constantBuffer);
				m_constantBuffer = NULL;
			}

			m_numPredefined = 0;

			if (NULL != m_buffer)
			{
				DX_RELEASE(m_buffer, 0);
			}

			DX_RELEASE(m_ptr, 0);

			if (NULL != m_code)
			{
				release(m_code);
				m_code = NULL;
				m_hash = 0;
			}
		}

		union
		{
			ID3D11ComputeShader* m_computeShader;
			ID3D11PixelShader*   m_pixelShader;
			ID3D11VertexShader*  m_vertexShader;
			ID3D11DeviceChild*   m_ptr;
		};
		const Memory* m_code;
		ID3D11Buffer* m_buffer;
		UniformBuffer* m_constantBuffer;

		PredefinedUniform m_predefined[PredefinedUniform::Count];
		uint16_t m_attrMask[Attrib::Count];

		uint32_t m_hash;

		uint16_t m_numUniforms;
		uint8_t m_numPredefined;
		bool m_hasDepthOp;
	};

	struct ProgramD3D11
	{
		ProgramD3D11()
			: m_vsh(NULL)
			, m_fsh(NULL)
		{
		}

		void create(const ShaderD3D11* _vsh, const ShaderD3D11* _fsh)
		{
			BASE_ASSERT(NULL != _vsh->m_ptr, "Vertex shader doesn't exist.");
			m_vsh = _vsh;
			base::memCopy(&m_predefined[0], _vsh->m_predefined, _vsh->m_numPredefined*sizeof(PredefinedUniform) );
			m_numPredefined = _vsh->m_numPredefined;

			if (NULL != _fsh)
			{
				BASE_ASSERT(NULL != _fsh->m_ptr, "Fragment shader doesn't exist.");
				m_fsh = _fsh;
				base::memCopy(&m_predefined[m_numPredefined], _fsh->m_predefined, _fsh->m_numPredefined*sizeof(PredefinedUniform) );
				m_numPredefined += _fsh->m_numPredefined;
			}
		}

		void destroy()
		{
			m_numPredefined = 0;
			m_vsh = NULL;
			m_fsh = NULL;
		}

		const ShaderD3D11* m_vsh;
		const ShaderD3D11* m_fsh;

		PredefinedUniform m_predefined[PredefinedUniform::Count*2];
		uint8_t m_numPredefined;
	};

	struct IntelDirectAccessResourceDescriptor
	{
		void*    ptr;
		uint32_t xoffset;
		uint32_t yoffset;
		uint32_t tileFormat;
		uint32_t pitch;
		uint32_t size;
	};

	struct DirectAccessResourceD3D11
	{
		DirectAccessResourceD3D11()
			: m_ptr(NULL)
			, m_descriptor(NULL)
		{
		}

		void* createTexture2D(const D3D11_TEXTURE2D_DESC* _gpuDesc, const D3D11_SUBRESOURCE_DATA* _srd, ID3D11Texture2D** _gpuTexture2d);
		void* createTexture3D(const D3D11_TEXTURE3D_DESC* _gpuDesc, const D3D11_SUBRESOURCE_DATA* _srd, ID3D11Texture3D** _gpuTexture3d);
		void destroy();

		union
		{
			ID3D11Resource*  m_ptr;
			ID3D11Texture2D* m_texture2d;
			ID3D11Texture3D* m_texture3d;
		};

		IntelDirectAccessResourceDescriptor* m_descriptor;
	};

	struct TextureD3D11
	{
		enum Enum
		{
			Texture2D,
			Texture3D,
			TextureCube,
		};

		TextureD3D11()
			: m_ptr(NULL)
			, m_rt(NULL)
			, m_srv(NULL)
			, m_uav(NULL)
			, m_numMips(0)
		{
		}

		void* create(const Memory* _mem, uint64_t _flags, uint8_t _skip);
		void destroy();
		void overrideInternal(uintptr_t _ptr);
		void update(uint8_t _side, uint8_t _mip, const Rect& _rect, uint16_t _z, uint16_t _depth, uint16_t _pitch, const Memory* _mem);
		void commit(uint8_t _stage, uint32_t _flags, const float _palette[][4]);
		void resolve(uint8_t _resolve, uint32_t _layer, uint32_t _numLayers, uint32_t _mip) const;
		TextureHandle getHandle() const;
		DXGI_FORMAT getSrvFormat() const;

		union
		{
			ID3D11Resource*  m_ptr;
			ID3D11Texture2D* m_texture2d;
			ID3D11Texture3D* m_texture3d;
		};

		DirectAccessResourceD3D11 m_dar;

		union
		{
			ID3D11Resource* m_rt;
			ID3D11Texture2D* m_rt2d;
		};

		ID3D11ShaderResourceView*  m_srv;
		ID3D11UnorderedAccessView* m_uav;
		uint64_t m_flags;
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_depth;
		uint32_t m_numLayers;
		uint8_t  m_type;
		uint8_t  m_requestedFormat;
		uint8_t  m_textureFormat;
		uint8_t  m_numMips;
	};

	struct FrameBufferD3D11
	{
		FrameBufferD3D11()
			: m_dsv(NULL)
			, m_swapChain(NULL)
			, m_nwh(NULL)
			, m_width(0)
			, m_height(0)
			, m_denseIdx(UINT16_MAX)
			, m_num(0)
			, m_numTh(0)
			, m_numUav(0)
			, m_needPresent(false)
		{
		}

		void create(uint8_t _num, const Attachment* _attachment);
		void create(uint16_t _denseIdx, void* _nwh, uint32_t _width, uint32_t _height, TextureFormat::Enum _format, TextureFormat::Enum _depthFormat);
		uint16_t destroy();
		void preReset(bool _force = false);
		void postReset();
		void resolve();
		void clear(const Clear& _clear, const float _palette[][4]);
		void set();
		HRESULT present(uint32_t _syncInterval);

		ID3D11RenderTargetView*    m_rtv[GRAPHICS_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS-1];
		ID3D11UnorderedAccessView* m_uav[GRAPHICS_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS-1];
		ID3D11ShaderResourceView*  m_srv[GRAPHICS_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS-1];
		ID3D11DepthStencilView*    m_dsv;
		Dxgi::SwapChainI* m_swapChain;
		void* m_nwh;
		uint32_t m_width;
		uint32_t m_height;

		Attachment m_attachment[GRAPHICS_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS];
		uint16_t m_denseIdx;
		uint8_t m_num;
		uint8_t m_numTh;
		uint8_t m_numUav;
		bool m_needPresent;
	};

	struct TimerQueryD3D11
	{
		TimerQueryD3D11()
			: m_control(BASE_COUNTOF(m_query) )
		{
		}

		void postReset();
		void preReset();
		uint32_t begin(uint32_t _resultIdx, uint32_t _frameNum);
		void end(uint32_t _idx);
		bool update();

		struct Query
		{
			ID3D11Query* m_disjoint;
			ID3D11Query* m_begin;
			ID3D11Query* m_end;
			uint32_t     m_resultIdx;
			uint32_t     m_frameNum;
			bool         m_ready;
		};

		struct Result
		{
			void reset()
			{
				m_begin     = 0;
				m_end       = 0;
				m_frequency = 1;
				m_pending   = 0;
				m_frameNum  = 0;
			}

			uint64_t m_begin;
			uint64_t m_end;
			uint64_t m_frequency;
			uint32_t m_pending;
			uint32_t m_frameNum;
		};

		Result m_result[GRAPHICS_CONFIG_MAX_VIEWS+1];

		Query m_query[GRAPHICS_CONFIG_MAX_VIEWS*4];
		base::RingBufferControl m_control;
	};

	struct OcclusionQueryD3D11
	{
		OcclusionQueryD3D11()
			: m_control(BASE_COUNTOF(m_query) )
		{
		}

		void postReset();
		void preReset();
		void begin(Frame* _render, OcclusionQueryHandle _handle);
		void end();
		void resolve(Frame* _render, bool _wait = false);
		void invalidate(OcclusionQueryHandle _handle);

		struct Query
		{
			ID3D11Query* m_ptr;
			OcclusionQueryHandle m_handle;
		};

		Query m_query[GRAPHICS_CONFIG_MAX_OCCLUSION_QUERIES];
		base::RingBufferControl m_control;
	};

} /*  namespace d3d11 */ } // namespace graphics

#endif // GRAPHICS_RENDERER_D3D11_H_HEADER_GUARD
