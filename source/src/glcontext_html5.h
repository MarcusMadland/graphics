/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#ifndef GRAPHICS_GLCONTEXT_HTML5_H_HEADER_GUARD
#define GRAPHICS_GLCONTEXT_HTML5_H_HEADER_GUARD

#if GRAPHICS_USE_HTML5

namespace graphics { namespace gl
{
	struct SwapChainGL;

	struct GlContext
	{
		GlContext()
			: m_current(NULL)
			, m_primary(NULL)
			, m_msaaContext(false)
		{
		}

		void create(uint32_t _width, uint32_t _height, uint32_t _flags);
		void destroy();
		void resize(uint32_t _width, uint32_t _height, uint32_t _flags);

		uint64_t getCaps() const;
		SwapChainGL* createSwapChain(void* _nwh);
		void destroySwapChain(SwapChainGL*  _swapChain);
		void swap(SwapChainGL* _swapChain = NULL);
		void makeCurrent(SwapChainGL* _swapChain = NULL);

		void import(int webGLVersion);

		bool isValid() const
		{
			return NULL != m_primary;
		}

        SwapChainGL* m_current;
		SwapChainGL* m_primary;
		// true when MSAA is handled by the context instead of using MSAA FBO
		bool m_msaaContext;
	};
} /* namespace gl */ } // namespace graphics

#endif // GRAPHICS_USE_HTML5

#endif // GRAPHICS_GLCONTEXT_HTML5_H_HEADER_GUARD
