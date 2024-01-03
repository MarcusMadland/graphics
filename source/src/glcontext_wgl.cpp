/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#include "graphics_p.h"

#if (GRAPHICS_CONFIG_RENDERER_OPENGLES|GRAPHICS_CONFIG_RENDERER_OPENGL)
#	include "renderer_gl.h"

#	if GRAPHICS_USE_WGL

namespace graphics { namespace gl
{
	PFNWGLGETPROCADDRESSPROC wglGetProcAddress;
	PFNWGLMAKECURRENTPROC wglMakeCurrent;
	PFNWGLCREATECONTEXTPROC wglCreateContext;
	PFNWGLDELETECONTEXTPROC wglDeleteContext;
	PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

#	define GL_IMPORT(_optional, _proto, _func, _import) _proto _func
#	include "glimports.h"
#	undef GL_IMPORT

	template<typename ProtoT>
	static ProtoT wglGetProc(const char* _name)
	{
		return reinterpret_cast<ProtoT>( (void*)wglGetProcAddress(_name) );
	}

	struct SwapChainGL
	{
		SwapChainGL(void* _nwh)
			: m_hwnd( (HWND)_nwh)
		{
			m_hdc = GetDC(m_hwnd);
		}

		~SwapChainGL()
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(m_context);
			ReleaseDC(m_hwnd, m_hdc);
		}

		void makeCurrent()
		{
			wglMakeCurrent(m_hdc, m_context);
			GLenum err = glGetError();
			BASE_WARN(0 == err, "wglMakeCurrent failed with GL error: 0x%04x.", err); BASE_UNUSED(err);
		}

		void swapBuffers()
		{
			SwapBuffers(m_hdc);
		}

		HWND  m_hwnd;
		HDC   m_hdc;
		HGLRC m_context;
	};

	static HGLRC createContext(HDC _hdc)
	{
		PIXELFORMATDESCRIPTOR pfd;
		base::memSet(&pfd, 0, sizeof(pfd) );
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cAlphaBits = 8;
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int pixelFormat = ChoosePixelFormat(_hdc, &pfd);
		GRAPHICS_FATAL(0 != pixelFormat, Fatal::UnableToInitialize, "ChoosePixelFormat failed!");

		DescribePixelFormat(_hdc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

		BASE_TRACE("Pixel format:\n"
			"\tiPixelType %d\n"
			"\tcColorBits %d\n"
			"\tcAlphaBits %d\n"
			"\tcDepthBits %d\n"
			"\tcStencilBits %d\n"
			, pfd.iPixelType
			, pfd.cColorBits
			, pfd.cAlphaBits
			, pfd.cDepthBits
			, pfd.cStencilBits
			);

		int result;
		result = SetPixelFormat(_hdc, pixelFormat, &pfd);
		GRAPHICS_FATAL(0 != result, Fatal::UnableToInitialize, "SetPixelFormat failed!");

		HGLRC context = wglCreateContext(_hdc);
		GRAPHICS_FATAL(NULL != context, Fatal::UnableToInitialize, "wglCreateContext failed!");

		result = wglMakeCurrent(_hdc, context);
		GRAPHICS_FATAL(0 != result, Fatal::UnableToInitialize, "wglMakeCurrent failed!");

		return context;
	}

	void GlContext::create(uint32_t /*_width*/, uint32_t /*_height*/, uint32_t /*_flags*/)
	{
		m_opengl32dll = base::dlopen("opengl32.dll");
		GRAPHICS_FATAL(NULL != m_opengl32dll, Fatal::UnableToInitialize, "Failed to load opengl32.dll.");

		wglGetProcAddress = base::dlsym<PFNWGLGETPROCADDRESSPROC>(m_opengl32dll, "wglGetProcAddress");
		GRAPHICS_FATAL(NULL != wglGetProcAddress, Fatal::UnableToInitialize, "Failed get wglGetProcAddress.");

		// If g_platformHooks.nwh is NULL, the assumption is that GL context was created
		// by user (for example, using SDL, GLFW, etc.)
		BASE_WARN(NULL != g_platformData.nwh
			, "graphics::setPlatform with valid window is not called. This might "
				"be intentional when GL context is created by the user."
			);

		if (NULL != g_platformData.nwh && NULL != g_platformData.context )
		{
			// user has provided a context and a window
			wglMakeCurrent = (PFNWGLMAKECURRENTPROC)base::dlsym(m_opengl32dll, "wglMakeCurrent");
			GRAPHICS_FATAL(NULL != wglMakeCurrent, Fatal::UnableToInitialize, "Failed get wglMakeCurrent.");

			m_hdc = GetDC( (HWND)g_platformData.nwh);
			GRAPHICS_FATAL(NULL != m_hdc, Fatal::UnableToInitialize, "GetDC failed!");

			HGLRC context = (HGLRC)g_platformData.context;
			int result = wglMakeCurrent(m_hdc, context );
			GRAPHICS_FATAL(0 != result, Fatal::UnableToInitialize, "wglMakeCurrent failed!");

			m_context = context;
		}

		if (NULL != g_platformData.nwh && NULL == g_platformData.context )
		{
			wglMakeCurrent = base::dlsym<PFNWGLMAKECURRENTPROC>(m_opengl32dll, "wglMakeCurrent");
			GRAPHICS_FATAL(NULL != wglMakeCurrent, Fatal::UnableToInitialize, "Failed get wglMakeCurrent.");

			wglCreateContext = base::dlsym<PFNWGLCREATECONTEXTPROC>(m_opengl32dll, "wglCreateContext");
			GRAPHICS_FATAL(NULL != wglCreateContext, Fatal::UnableToInitialize, "Failed get wglCreateContext.");

			wglDeleteContext = base::dlsym<PFNWGLDELETECONTEXTPROC>(m_opengl32dll, "wglDeleteContext");
			GRAPHICS_FATAL(NULL != wglDeleteContext, Fatal::UnableToInitialize, "Failed get wglDeleteContext.");

			m_hdc = GetDC( (HWND)g_platformData.nwh);
			GRAPHICS_FATAL(NULL != m_hdc, Fatal::UnableToInitialize, "GetDC failed!");

			// Dummy window to peek into WGL functionality.
			//
			// An application can only set the pixel format of a window one time.
			// Once a window's pixel format is set, it cannot be changed.
			// MSDN: https://web.archive.org/web/20190207230357/https://docs.microsoft.com/en-us/windows/desktop/api/wingdi/nf-wingdi-setpixelformat
			HWND hwnd = CreateWindowA("STATIC"
				, ""
				, WS_POPUP|WS_DISABLED
				, -32000
				, -32000
				, 0
				, 0
				, NULL
				, NULL
				, GetModuleHandle(NULL)
				, 0
				);

			HDC hdc = GetDC(hwnd);
			GRAPHICS_FATAL(NULL != hdc, Fatal::UnableToInitialize, "GetDC failed!");

			HGLRC context = createContext(hdc);

			wglGetExtensionsStringARB  = wglGetProc<PFNWGLGETEXTENSIONSSTRINGARBPROC >("wglGetExtensionsStringARB");
			wglChoosePixelFormatARB    = wglGetProc<PFNWGLCHOOSEPIXELFORMATARBPROC   >("wglChoosePixelFormatARB");
			wglCreateContextAttribsARB = wglGetProc<PFNWGLCREATECONTEXTATTRIBSARBPROC>("wglCreateContextAttribsARB");
			wglSwapIntervalEXT         = wglGetProc<PFNWGLSWAPINTERVALEXTPROC        >("wglSwapIntervalEXT");

			if (NULL != wglGetExtensionsStringARB)
			{
				const char* extensions = (const char*)wglGetExtensionsStringARB(hdc);
				BASE_TRACE("WGL extensions:");
				dumpExtensions(extensions);
			}

			if (NULL != wglChoosePixelFormatARB
			&&  NULL != wglCreateContextAttribsARB)
			{
				int32_t attrs[] =
				{
					WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
					WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
					WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
					WGL_SUPPORT_OPENGL_ARB, GL_TRUE,

					WGL_ALPHA_BITS_ARB,     8,
					WGL_COLOR_BITS_ARB,     32,
					WGL_DEPTH_BITS_ARB,     24,
					WGL_STENCIL_BITS_ARB,   8,

					WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
					WGL_SAMPLES_ARB,        0,
					WGL_SAMPLE_BUFFERS_ARB, GL_FALSE,

					0
				};

				int result;
				uint32_t numFormats = 0;
				do
				{
					result = wglChoosePixelFormatARB(m_hdc, attrs, NULL, 1, &m_pixelFormat, &numFormats);
					if (0 == result
					||  0 == numFormats)
					{
						attrs[3] >>= 1;
						attrs[1] = attrs[3] == 0 ? 0 : 1;
					}

				} while (0 == numFormats);

				DescribePixelFormat(m_hdc, m_pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &m_pfd);

				BASE_TRACE("Pixel format:\n"
					"\tiPixelType %d\n"
					"\tcColorBits %d\n"
					"\tcAlphaBits %d\n"
					"\tcDepthBits %d\n"
					"\tcStencilBits %d\n"
					, m_pfd.iPixelType
					, m_pfd.cColorBits
					, m_pfd.cAlphaBits
					, m_pfd.cDepthBits
					, m_pfd.cStencilBits
					);

				result = SetPixelFormat(m_hdc, m_pixelFormat, &m_pfd);
				// When window is created by SDL and SDL_WINDOW_OPENGL is set, SetPixelFormat
				// will fail. Just warn and continue. In case it failed for some other reason
				// create context will fail and it will error out there.
				BASE_WARN(result, "SetPixelFormat failed (last err: 0x%08x)!", GetLastError() );

				int32_t flags = GRAPHICS_CONFIG_DEBUG ? WGL_CONTEXT_DEBUG_BIT_ARB : 0;
				BASE_UNUSED(flags);
				int32_t contextAttrs[9] =
				{
#if GRAPHICS_CONFIG_RENDERER_OPENGL >= 31
					WGL_CONTEXT_MAJOR_VERSION_ARB, GRAPHICS_CONFIG_RENDERER_OPENGL / 10,
					WGL_CONTEXT_MINOR_VERSION_ARB, GRAPHICS_CONFIG_RENDERER_OPENGL % 10,
					WGL_CONTEXT_FLAGS_ARB, flags,
					WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#else
					WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
					WGL_CONTEXT_MINOR_VERSION_ARB, 1,
					0, 0,
					0, 0,
#endif // GRAPHICS_CONFIG_RENDERER_OPENGL >= 31
					0
				};

				m_context = wglCreateContextAttribsARB(m_hdc, 0, contextAttrs);
				if (NULL == m_context)
				{
					// nVidia doesn't like context profile mask for contexts below 3.2?
					contextAttrs[6] = WGL_CONTEXT_PROFILE_MASK_ARB == contextAttrs[6] ? 0 : contextAttrs[6];
					m_context = wglCreateContextAttribsARB(m_hdc, 0, contextAttrs);
				}
				GRAPHICS_FATAL(NULL != m_context, Fatal::UnableToInitialize, "Failed to create context 0x%08x.", GetLastError() );

				BASE_STATIC_ASSERT(sizeof(contextAttrs) == sizeof(m_contextAttrs) );
				base::memCopy(m_contextAttrs, contextAttrs, sizeof(contextAttrs) );
			}

			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(context);
			DestroyWindow(hwnd);

			if (NULL == m_context)
			{
				m_context = createContext(m_hdc);
			}

			int result = wglMakeCurrent(m_hdc, m_context);
			GRAPHICS_FATAL(0 != result, Fatal::UnableToInitialize, "wglMakeCurrent failed!");
			m_current = NULL;

			if (NULL != wglSwapIntervalEXT)
			{
				wglSwapIntervalEXT(0);
			}
		}

		import();

		g_internalData.context = m_context;
	}

	void GlContext::destroy()
	{
		if (NULL != g_platformData.nwh)
		{
			wglMakeCurrent(NULL, NULL);

			if (NULL == g_platformData.context)
			{
				wglDeleteContext(m_context);
				m_context = NULL;

			}

			ReleaseDC( (HWND)g_platformData.nwh, m_hdc);
			m_hdc = NULL;
		}

		base::dlclose(m_opengl32dll);
		m_opengl32dll = NULL;
	}

	void GlContext::resize(uint32_t /*_width*/, uint32_t /*_height*/, uint32_t _flags)
	{
		if (NULL != wglSwapIntervalEXT)
		{
			bool vsync = !!(_flags&GRAPHICS_RESET_VSYNC);
			wglSwapIntervalEXT(vsync ? 1 : 0);
		}
	}

	uint64_t GlContext::getCaps() const
	{
		return GRAPHICS_CAPS_SWAP_CHAIN;
	}

	SwapChainGL* GlContext::createSwapChain(void* _nwh)
	{
		SwapChainGL* swapChain = BASE_NEW(g_allocator, SwapChainGL)(_nwh);

		int result = SetPixelFormat(swapChain->m_hdc, m_pixelFormat, &m_pfd);
		BASE_WARN(result, "SetPixelFormat failed (last err: 0x%08x)!", GetLastError() ); BASE_UNUSED(result);

		swapChain->m_context = wglCreateContextAttribsARB(swapChain->m_hdc, m_context, m_contextAttrs);
		BASE_ASSERT(NULL != swapChain->m_context, "Create swap chain failed: %x", glGetError() );
		return swapChain;
	}

	void GlContext::destroySwapChain(SwapChainGL*  _swapChain)
	{
		base::deleteObject(g_allocator, _swapChain);
		wglMakeCurrent(m_hdc, m_context);
	}

	void GlContext::swap(SwapChainGL* _swapChain)
	{
		makeCurrent(_swapChain);

		if (NULL == _swapChain)
		{
			if (NULL != g_platformData.nwh)
			{
				SwapBuffers(m_hdc);
			}
		}
		else
		{
			_swapChain->swapBuffers();
		}
	}

	void GlContext::makeCurrent(SwapChainGL* _swapChain)
	{
		if (m_current != _swapChain)
		{
			m_current = _swapChain;

			if (NULL == _swapChain)
			{
				wglMakeCurrent(m_hdc, m_context);
				GLenum err = glGetError();
				BASE_WARN(0 == err, "wglMakeCurrent failed with GL error: 0x%04x.", err); BASE_UNUSED(err);
			}
			else
			{
				_swapChain->makeCurrent();
			}
		}
	}

	void GlContext::import()
	{
		BASE_TRACE("Import:");

#	define GL_EXTENSION(_optional, _proto, _func, _import)                                     \
		{                                                                                      \
			if (NULL == _func)                                                                 \
			{                                                                                  \
				_func = wglGetProc<_proto>(#_import);                                          \
				if (NULL == _func)                                                             \
				{                                                                              \
					_func = base::dlsym<_proto>(m_opengl32dll, #_import);                        \
					BASE_TRACE("    %p " #_func " (" #_import ")", _func);                       \
				}                                                                              \
				else                                                                           \
				{                                                                              \
					BASE_TRACE("wgl %p " #_func " (" #_import ")", _func);                       \
				}                                                                              \
				GRAPHICS_FATAL(BASE_IGNORE_C4127(_optional) || NULL != _func                         \
					, Fatal::UnableToInitialize                                                \
					, "Failed to create OpenGL context. wglGetProcAddress(\"%s\")", #_import); \
			}                                                                                  \
		}

#	include "glimports.h"

#	undef GL_EXTENSION
	}

} } // namespace graphics

#	endif // GRAPHICS_USE_WGL
#endif // (GRAPHICS_CONFIG_RENDERER_OPENGLES|GRAPHICS_CONFIG_RENDERER_OPENGL)
