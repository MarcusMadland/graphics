/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#include "graphics_p.h"

#if (GRAPHICS_CONFIG_RENDERER_OPENGLES || GRAPHICS_CONFIG_RENDERER_OPENGL)
#	include "renderer_gl.h"

#	if GRAPHICS_USE_EGL

#		if BASE_PLATFORM_RPI
#			include <X11/Xlib.h>
#			include <bcm_host.h>
#		endif // BASE_PLATFORM_RPI

#define _EGL_CHECK(_check, _call)                                   \
	BASE_MACRO_BLOCK_BEGIN                                            \
		EGLBoolean success = _call;                                 \
		_check(success, #_call "; EGL error 0x%x", eglGetError() ); \
	BASE_MACRO_BLOCK_END

#if GRAPHICS_CONFIG_DEBUG
#	define EGL_CHECK(_call) _EGL_CHECK(BASE_ASSERT, _call)
#else
#	define EGL_CHECK(_call) _call
#endif // GRAPHICS_CONFIG_DEBUG

namespace graphics { namespace gl
{
#ifndef EGL_CONTEXT_FLAG_NO_ERROR_BIT_KHR
#	define EGL_CONTEXT_FLAG_NO_ERROR_BIT_KHR 0x00000008
#endif // EGL_CONTEXT_FLAG_NO_ERROR_BIT_KHR

#if GRAPHICS_USE_GL_DYNAMIC_LIB

	typedef void (*EGLPROC)(void);

	typedef EGLBoolean  (EGLAPIENTRY* PGNEGLBINDAPIPROC)(EGLenum api);
	typedef EGLBoolean  (EGLAPIENTRY* PFNEGLCHOOSECONFIGPROC)(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_config);
	typedef EGLContext  (EGLAPIENTRY* PFNEGLCREATECONTEXTPROC)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint* attrib_list);
	typedef EGLSurface  (EGLAPIENTRY* PFNEGLCREATEPBUFFERSURFACEPROC)(EGLDisplay display, EGLConfig config, EGLint const* attrib_list);
	typedef EGLSurface  (EGLAPIENTRY* PFNEGLCREATEWINDOWSURFACEPROC)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint* attrib_list);
	typedef EGLBoolean  (EGLAPIENTRY* PFNEGLDESTROYCONTEXTPROC)(EGLDisplay dpy, EGLContext ctx);
	typedef EGLBoolean  (EGLAPIENTRY* PFNEGLDESTROYSURFACEPROC)(EGLDisplay dpy, EGLSurface surface);
	typedef EGLContext  (EGLAPIENTRY* PFNEGLGETCURRENTCONTEXTPROC)(void);
	typedef EGLSurface  (EGLAPIENTRY* PFNEGLGETCURRENTSURFACEPROC)(EGLint readdraw);
	typedef EGLDisplay  (EGLAPIENTRY* PFNEGLGETDISPLAYPROC)(EGLNativeDisplayType display_id);
	typedef EGLint      (EGLAPIENTRY* PFNEGLGETERRORPROC)(void);
	typedef EGLPROC     (EGLAPIENTRY* PFNEGLGETPROCADDRESSPROC)(const char* procname);
	typedef EGLBoolean  (EGLAPIENTRY* PFNEGLINITIALIZEPROC)(EGLDisplay dpy, EGLint* major, EGLint* minor);
	typedef EGLBoolean  (EGLAPIENTRY* PFNEGLMAKECURRENTPROC)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
	typedef EGLBoolean  (EGLAPIENTRY* PFNEGLSWAPBUFFERSPROC)(EGLDisplay dpy, EGLSurface surface);
	typedef EGLBoolean  (EGLAPIENTRY* PFNEGLSWAPINTERVALPROC)(EGLDisplay dpy, EGLint interval);
	typedef EGLBoolean  (EGLAPIENTRY* PFNEGLTERMINATEPROC)(EGLDisplay dpy);
	typedef const char* (EGLAPIENTRY* PGNEGLQUERYSTRINGPROC)(EGLDisplay dpy, EGLint name);

#define EGL_IMPORT                                                            \
	EGL_IMPORT_FUNC(PGNEGLBINDAPIPROC,              eglBindAPI);              \
	EGL_IMPORT_FUNC(PFNEGLCHOOSECONFIGPROC,         eglChooseConfig);         \
	EGL_IMPORT_FUNC(PFNEGLCREATECONTEXTPROC,        eglCreateContext);        \
	EGL_IMPORT_FUNC(PFNEGLCREATEPBUFFERSURFACEPROC, eglCreatePbufferSurface); \
	EGL_IMPORT_FUNC(PFNEGLCREATEWINDOWSURFACEPROC,  eglCreateWindowSurface);  \
	EGL_IMPORT_FUNC(PFNEGLDESTROYCONTEXTPROC,       eglDestroyContext);       \
	EGL_IMPORT_FUNC(PFNEGLDESTROYSURFACEPROC,       eglDestroySurface);       \
	EGL_IMPORT_FUNC(PFNEGLGETCURRENTCONTEXTPROC,    eglGetCurrentContext);    \
	EGL_IMPORT_FUNC(PFNEGLGETCURRENTSURFACEPROC,    eglGetCurrentSurface);    \
	EGL_IMPORT_FUNC(PFNEGLGETDISPLAYPROC,           eglGetDisplay);           \
	EGL_IMPORT_FUNC(PFNEGLGETERRORPROC,             eglGetError);             \
	EGL_IMPORT_FUNC(PFNEGLGETPROCADDRESSPROC,       eglGetProcAddress);       \
	EGL_IMPORT_FUNC(PFNEGLINITIALIZEPROC,           eglInitialize);           \
	EGL_IMPORT_FUNC(PFNEGLMAKECURRENTPROC,          eglMakeCurrent);          \
	EGL_IMPORT_FUNC(PFNEGLSWAPBUFFERSPROC,          eglSwapBuffers);          \
	EGL_IMPORT_FUNC(PFNEGLSWAPINTERVALPROC,         eglSwapInterval);         \
	EGL_IMPORT_FUNC(PFNEGLTERMINATEPROC,            eglTerminate);            \
	EGL_IMPORT_FUNC(PGNEGLQUERYSTRINGPROC,          eglQueryString);          \

#define EGL_IMPORT_FUNC(_proto, _func) _proto _func
EGL_IMPORT
#undef EGL_IMPORT_FUNC

	void* eglOpen()
	{
	    void* handle = base::dlopen(
#if BASE_PLATFORM_LINUX
			"libEGL.so.1"
#else
			"libEGL." BASE_DL_EXT
#endif // BASE_PLATFORM_*
			);

		GRAPHICS_FATAL(NULL != handle, Fatal::UnableToInitialize, "Failed to load libEGL dynamic library.");

#define EGL_IMPORT_FUNC(_proto, _func)         \
	_func = (_proto)base::dlsym(handle, #_func); \
	BASE_TRACE("%p " #_func, _func);             \
	GRAPHICS_FATAL(NULL != _func, Fatal::UnableToInitialize, "Failed get " #_func ".")
EGL_IMPORT
#undef EGL_IMPORT_FUNC

		return handle;
	}

	void eglClose(void* _handle)
	{
		base::dlclose(_handle);

#define EGL_IMPORT_FUNC(_proto, _func) _func = NULL
EGL_IMPORT
#undef EGL_IMPORT_FUNC
	}

#else

	void* eglOpen()
	{
		return NULL;
	}

	void eglClose(void* /*_handle*/)
	{
	}
#endif // GRAPHICS_USE_GL_DYNAMIC_LIB

#	define GL_IMPORT(_optional, _proto, _func, _import) _proto _func = NULL
#	include "glimports.h"

	static EGLint s_contextAttrs[16];

	struct SwapChainGL
	{
		SwapChainGL(EGLDisplay _display, EGLConfig _config, EGLContext _context, EGLNativeWindowType _nwh)
			: m_nwh(_nwh)
			, m_display(_display)
		{
			EGLSurface defaultSurface = eglGetCurrentSurface(EGL_DRAW);

			if (EGLNativeWindowType(0) == _nwh)
			{
				m_surface = eglCreatePbufferSurface(m_display, _config, NULL);
			}
			else
			{
				m_surface = eglCreateWindowSurface(m_display, _config, _nwh, NULL);
			}

			GRAPHICS_FATAL(m_surface != EGL_NO_SURFACE, Fatal::UnableToInitialize, "Failed to create surface.");

			m_context = eglCreateContext(m_display, _config, _context, s_contextAttrs);
			BASE_ASSERT(NULL != m_context, "Create swap chain failed: %x", eglGetError() );

			makeCurrent();
			GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f) );
			GL_CHECK(glClear(GL_COLOR_BUFFER_BIT) );
			swapBuffers();

			GL_CHECK(glClear(GL_COLOR_BUFFER_BIT) );
			swapBuffers();

			EGL_CHECK(eglMakeCurrent(m_display, defaultSurface, defaultSurface, _context) );
		}

		~SwapChainGL()
		{
			EGLSurface defaultSurface = eglGetCurrentSurface(EGL_DRAW);
			EGLContext defaultContext = eglGetCurrentContext();

			EGL_CHECK(eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) );
			EGL_CHECK(eglDestroyContext(m_display, m_context) );
			EGL_CHECK(eglDestroySurface(m_display, m_surface) );
			EGL_CHECK(eglMakeCurrent(m_display, defaultSurface, defaultSurface, defaultContext) );
		}

		void makeCurrent()
		{
			EGL_CHECK(eglMakeCurrent(m_display, m_surface, m_surface, m_context) );
		}

		void swapBuffers()
		{
			EGL_CHECK(eglSwapBuffers(m_display, m_surface) );
		}

		EGLNativeWindowType m_nwh;
		EGLContext m_context;
		EGLDisplay m_display;
		EGLSurface m_surface;
	};

#	if BASE_PLATFORM_RPI
	static EGL_DISPMANX_WINDOW_T s_dispmanWindow;
#	endif // BASE_PLATFORM_RPI

	void GlContext::create(uint32_t _width, uint32_t _height, uint32_t _flags)
	{
		BASE_UNUSED(_flags);

#	if BASE_PLATFORM_RPI
		bcm_host_init();
#	endif // BASE_PLATFORM_RPI

		m_eglLibrary = eglOpen();

		if (NULL == g_platformData.context)
		{
#	if BASE_PLATFORM_RPI
			g_platformData.ndt = EGL_DEFAULT_DISPLAY;
#	endif // BASE_PLATFORM_RPI

			BASE_UNUSED(_width, _height);
			EGLNativeDisplayType ndt = (EGLNativeDisplayType)g_platformData.ndt;
			EGLNativeWindowType  nwh = (EGLNativeWindowType )g_platformData.nwh;

#	if BASE_PLATFORM_WINDOWS
			if (NULL == g_platformData.ndt)
			{
				ndt = GetDC( (HWND)g_platformData.nwh);
			}
#	endif // BASE_PLATFORM_WINDOWS

            m_display = eglGetDisplay(NULL == ndt ? EGL_DEFAULT_DISPLAY : ndt);
			GRAPHICS_FATAL(m_display != EGL_NO_DISPLAY, Fatal::UnableToInitialize, "Failed to create display %p", m_display);

			EGLint major = 0;
			EGLint minor = 0;
			EGLBoolean success = eglInitialize(m_display, &major, &minor);
			GRAPHICS_FATAL(success && major >= 1 && minor >= 3, Fatal::UnableToInitialize, "Failed to initialize %d.%d", major, minor);

			BASE_TRACE("EGL info:");
			const char* clientApis = eglQueryString(m_display, EGL_CLIENT_APIS);
			BASE_TRACE("   APIs: %s", clientApis); BASE_UNUSED(clientApis);

			const char* vendor = eglQueryString(m_display, EGL_VENDOR);
			BASE_TRACE(" Vendor: %s", vendor); BASE_UNUSED(vendor);

			const char* version = eglQueryString(m_display, EGL_VERSION);
			BASE_TRACE("Version: %s", version); BASE_UNUSED(version);

			const char* extensions = eglQueryString(m_display, EGL_EXTENSIONS);
			BASE_TRACE("Supported EGL extensions:");
			dumpExtensions(extensions);

			if (BASE_ENABLED(GRAPHICS_CONFIG_RENDERER_OPENGL) )
			{
				EGLBoolean ok = eglBindAPI(EGL_OPENGL_API);
				GRAPHICS_FATAL(ok, Fatal::UnableToInitialize, "Could not set API! error: %d", eglGetError());
			}

			const bool hasEglAndroidRecordable = !base::findIdentifierMatch(extensions, "EGL_ANDROID_recordable").isEmpty();

			const uint32_t glVersion = !!GRAPHICS_CONFIG_RENDERER_OPENGL
				? GRAPHICS_CONFIG_RENDERER_OPENGL
				: GRAPHICS_CONFIG_RENDERER_OPENGLES
				;

#if BASE_PLATFORM_ANDROID
			const uint32_t msaa = (_flags&GRAPHICS_RESET_MSAA_MASK)>>GRAPHICS_RESET_MSAA_SHIFT;
			const uint32_t msaaSamples = msaa == 0 ? 0 : 1<<msaa;
			m_msaaContext = true;
#endif // BASE_PLATFORM_ANDROID

			const bool headless = EGLNativeWindowType(0) == nwh;

			EGLint attrs[] =
			{
				EGL_RENDERABLE_TYPE, !!GRAPHICS_CONFIG_RENDERER_OPENGL
					? EGL_OPENGL_BIT
					: (glVersion >= 30) ? EGL_OPENGL_ES3_BIT_KHR : EGL_OPENGL_ES2_BIT
					,

				EGL_SURFACE_TYPE, headless ? EGL_PBUFFER_BIT : EGL_WINDOW_BIT,

				EGL_BLUE_SIZE,  8,
				EGL_GREEN_SIZE, 8,
				EGL_RED_SIZE,   8,
				EGL_ALPHA_SIZE, 8,

#	if BASE_PLATFORM_ANDROID
				EGL_DEPTH_SIZE, 16,
				EGL_SAMPLES, (EGLint)msaaSamples,
#	else
				EGL_DEPTH_SIZE, 24,
#	endif // BASE_PLATFORM_
				EGL_STENCIL_SIZE, 8,

				// Android Recordable surface
				hasEglAndroidRecordable ? EGL_RECORDABLE_ANDROID : EGL_NONE,
				hasEglAndroidRecordable ? 1                      : EGL_NONE,

				EGL_NONE
			};

			EGLint numConfig = 0;
			success = eglChooseConfig(m_display, attrs, &m_config, 1, &numConfig);
			GRAPHICS_FATAL(success, Fatal::UnableToInitialize, "eglChooseConfig");

#	if BASE_PLATFORM_ANDROID

			EGLint format;
			eglGetConfigAttrib(m_display, m_config, EGL_NATIVE_VISUAL_ID, &format);
			ANativeWindow_setBuffersGeometry( (ANativeWindow*)g_platformData.nwh, _width, _height, format);

#	elif BASE_PLATFORM_RPI
			DISPMANX_DISPLAY_HANDLE_T dispmanDisplay = vc_dispmanx_display_open(0);
			DISPMANX_UPDATE_HANDLE_T  dispmanUpdate  = vc_dispmanx_update_start(0);

			VC_RECT_T dstRect = { 0, 0, int32_t(_width),        int32_t(_height)       };
			VC_RECT_T srcRect = { 0, 0, int32_t(_width)  << 16, int32_t(_height) << 16 };

			DISPMANX_ELEMENT_HANDLE_T dispmanElement = vc_dispmanx_element_add(dispmanUpdate
				, dispmanDisplay
				, 0
				, &dstRect
				, 0
				, &srcRect
				, DISPMANX_PROTECTION_NONE
				, NULL
				, NULL
				, DISPMANX_NO_ROTATE
				);

			s_dispmanWindow.element = dispmanElement;
			s_dispmanWindow.width   = _width;
			s_dispmanWindow.height  = _height;
			nwh = &s_dispmanWindow;

			vc_dispmanx_update_submit_sync(dispmanUpdate);
#	endif // BASE_PLATFORM_ANDROID

			if (headless)
			{
				EGLint pbAttribs[] =
				{
					EGL_WIDTH,  EGLint(1),
					EGL_HEIGHT, EGLint(1),

					EGL_NONE
				};

				m_surface = eglCreatePbufferSurface(m_display, m_config, pbAttribs);
			}
			else
			{
				m_surface = eglCreateWindowSurface(m_display, m_config, nwh, NULL);
			}

			GRAPHICS_FATAL(m_surface != EGL_NO_SURFACE, Fatal::UnableToInitialize, "Failed to create surface.");

			const bool hasEglKhrCreateContext = !base::findIdentifierMatch(extensions, "EGL_KHR_create_context").isEmpty();
			const bool hasEglKhrNoError       = !base::findIdentifierMatch(extensions, "EGL_KHR_create_context_no_error").isEmpty();

			for (uint32_t ii = 0; ii < 2; ++ii)
			{
				base::StaticMemoryBlockWriter writer(s_contextAttrs, sizeof(s_contextAttrs) );

				EGLint flags = 0;

#	if BASE_PLATFORM_RPI
				BASE_UNUSED(hasEglKhrCreateContext, hasEglKhrNoError);
#	else
				if (hasEglKhrCreateContext)
				{
					if (BASE_ENABLED(GRAPHICS_CONFIG_RENDERER_OPENGL) )
					{
						base::write(&writer, EGLint(EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR), base::ErrorAssert{});
						base::write(&writer, EGLint(EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR), base::ErrorAssert{});
					}

					base::write(&writer, EGLint(EGL_CONTEXT_MAJOR_VERSION_KHR), base::ErrorAssert{});
					base::write(&writer, EGLint(glVersion / 10), base::ErrorAssert{});

					base::write(&writer, EGLint(EGL_CONTEXT_MINOR_VERSION_KHR), base::ErrorAssert{});
					base::write(&writer, EGLint(glVersion % 10), base::ErrorAssert{});

					flags |= GRAPHICS_CONFIG_DEBUG && hasEglKhrNoError ? 0
						| EGL_CONTEXT_FLAG_NO_ERROR_BIT_KHR
						: 0
						;

					if (0 == ii)
					{
						flags |= GRAPHICS_CONFIG_DEBUG ? 0
							| EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR
//							| EGL_OPENGL_ES3_BIT_KHR
							: 0
							;

						base::write(&writer, EGLint(EGL_CONTEXT_FLAGS_KHR), base::ErrorAssert{} );
						base::write(&writer, flags, base::ErrorAssert{});
					}
				}
				else
#	endif // BASE_PLATFORM_RPI
				{
					base::write(&writer, EGLint(EGL_CONTEXT_CLIENT_VERSION), base::ErrorAssert{} );
					base::write(&writer, EGLint(glVersion / 10), base::ErrorAssert{} );
				}

				base::write(&writer, EGLint(EGL_NONE), base::ErrorAssert{} );

				m_context = eglCreateContext(m_display, m_config, EGL_NO_CONTEXT, s_contextAttrs);
				if (NULL != m_context)
				{
					break;
				}

				BASE_TRACE("Failed to create EGL context with EGL_CONTEXT_FLAGS_KHR (%08x).", flags);
			}

			GRAPHICS_FATAL(m_context != EGL_NO_CONTEXT, Fatal::UnableToInitialize, "Failed to create context.");

			success = eglMakeCurrent(m_display, m_surface, m_surface, m_context);
			GRAPHICS_FATAL(success, Fatal::UnableToInitialize, "Failed to set context.");
			m_current = NULL;

			eglSwapInterval(m_display, 0);
		}

		import();

		g_internalData.context = m_context;
	}

	void GlContext::destroy()
	{
		if (NULL != m_display)
		{
			EGL_CHECK(eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) );
			EGL_CHECK(eglDestroyContext(m_display, m_context) );
			EGL_CHECK(eglDestroySurface(m_display, m_surface) );
			EGL_CHECK(eglTerminate(m_display) );
			m_context = NULL;
		}

		eglClose(m_eglLibrary);

#	if BASE_PLATFORM_RPI
		bcm_host_deinit();
#	endif // BASE_PLATFORM_RPI
	}

	void GlContext::resize(uint32_t _width, uint32_t _height, uint32_t _flags)
	{
#	if BASE_PLATFORM_ANDROID
		if (NULL != m_display)
		{
			EGLNativeWindowType nwh = (EGLNativeWindowType )g_platformData.nwh;
			eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglDestroySurface(m_display, m_surface);
			m_surface = eglCreateWindowSurface(m_display, m_config, nwh, NULL);
			GRAPHICS_FATAL(m_surface != EGL_NO_SURFACE, Fatal::UnableToInitialize, "Failed to create surface.");
			EGLBoolean success = eglMakeCurrent(m_display, m_surface, m_surface, m_context);
			GRAPHICS_FATAL(success, Fatal::UnableToInitialize, "Failed to set context.");

			EGLint format;
			eglGetConfigAttrib(m_display, m_config, EGL_NATIVE_VISUAL_ID, &format);
			ANativeWindow_setBuffersGeometry( (ANativeWindow*)g_platformData.nwh, _width, _height, format);
		}
#	elif BASE_PLATFORM_EMSCRIPTEN
		EMSCRIPTEN_CHECK(emscripten_set_canvas_element_size(HTML5_TARGET_CANVAS_SELECTOR, _width, _height) );
#	else
		BASE_UNUSED(_width, _height);
#	endif // BASE_PLATFORM_*

		if (NULL != m_display)
		{
			bool vsync = !!(_flags&GRAPHICS_RESET_VSYNC);
			EGL_CHECK(eglSwapInterval(m_display, vsync ? 1 : 0) );
		}
	}

	uint64_t GlContext::getCaps() const
	{
		return BASE_ENABLED(0
			| BASE_PLATFORM_LINUX
			| BASE_PLATFORM_WINDOWS
			| BASE_PLATFORM_ANDROID
			)
			? GRAPHICS_CAPS_SWAP_CHAIN
			: 0
			;
	}

	SwapChainGL* GlContext::createSwapChain(void* _nwh)
	{
		return BASE_NEW(g_allocator, SwapChainGL)(m_display, m_config, m_context, (EGLNativeWindowType)_nwh);
	}

	void GlContext::destroySwapChain(SwapChainGL* _swapChain)
	{
		base::deleteObject(g_allocator, _swapChain);
	}

	void GlContext::swap(SwapChainGL* _swapChain)
	{
		makeCurrent(_swapChain);

		if (NULL == _swapChain)
		{
			if (NULL != m_display)
			{
				EGL_CHECK(eglSwapBuffers(m_display, m_surface) );
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
				if (NULL != m_display)
				{
					EGL_CHECK(eglMakeCurrent(m_display, m_surface, m_surface, m_context) );
				}
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

#	if BASE_PLATFORM_WINDOWS || BASE_PLATFORM_LINUX
#		if BASE_PLATFORM_WINDOWS
#			define LIBRARY_NAME "libGL.dll"
#		elif BASE_PLATFORM_LINUX
#			if GRAPHICS_CONFIG_RENDERER_OPENGL
#				define LIBRARY_NAME "libGL.so.1"
#			else
#				define LIBRARY_NAME "libGLESv2.so.2"
#			endif
#		endif

		void* lib = base::dlopen(LIBRARY_NAME);

#		define GL_EXTENSION(_optional, _proto, _func, _import)                           \
			{                                                                            \
				if (NULL == _func)                                                       \
				{                                                                        \
					_func = base::dlsym<_proto>(lib, #_import);                            \
					BASE_TRACE("\t%p " #_func " (" #_import ")", _func);                   \
					GRAPHICS_FATAL(_optional || NULL != _func                                \
						, Fatal::UnableToInitialize                                      \
						, "Failed to create OpenGLES context. eglGetProcAddress(\"%s\")" \
						, #_import);                                                     \
				}                                                                        \
			}
#	else
#		define GL_EXTENSION(_optional, _proto, _func, _import)                           \
			{                                                                            \
				if (NULL == _func)                                                       \
				{                                                                        \
					_func = reinterpret_cast<_proto>(eglGetProcAddress(#_import) );      \
					BASE_TRACE("\t%p " #_func " (" #_import ")", _func);                   \
					GRAPHICS_FATAL(_optional || NULL != _func                                \
						, Fatal::UnableToInitialize                                      \
						, "Failed to create OpenGLES context. eglGetProcAddress(\"%s\")" \
						, #_import);                                                     \
				}                                                                        \
			}

#	endif // BASE_PLATFORM_

#	include "glimports.h"

#	undef GL_EXTENSION
	}

} /* namespace gl */ } // namespace graphics

#	endif // GRAPHICS_USE_EGL
#endif // (GRAPHICS_CONFIG_RENDERER_OPENGLES || GRAPHICS_CONFIG_RENDERER_OPENGL)
