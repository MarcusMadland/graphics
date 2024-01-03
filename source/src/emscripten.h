/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#ifndef GRAPHICS_EMSCRIPTEN_H_HEADER_GUARD
#define GRAPHICS_EMSCRIPTEN_H_HEADER_GUARD

#if BASE_PLATFORM_EMSCRIPTEN

#	include <emscripten/emscripten.h>
#	include <emscripten/html5.h>

#	define _EMSCRIPTEN_CHECK(_check, _call)                                                                   \
		BASE_MACRO_BLOCK_BEGIN                                                                                  \
			EMSCRIPTEN_RESULT __result__ = _call;                                                             \
			_check(EMSCRIPTEN_RESULT_SUCCESS == __result__, #_call " FAILED 0x%08x\n", (uint32_t)__result__); \
			BASE_UNUSED(__result__);                                                                            \
		BASE_MACRO_BLOCK_END

#	if GRAPHICS_CONFIG_DEBUG
#		define EMSCRIPTEN_CHECK(_call) _EMSCRIPTEN_CHECK(BASE_ASSERT, _call)
#	else
#		define EMSCRIPTEN_CHECK(_call) _call
#	endif // GRAPHICS_CONFIG_DEBUG

#	ifndef HTML5_TARGET_CANVAS_SELECTOR
#		define HTML5_TARGET_CANVAS_SELECTOR "#canvas"
#	endif // HTML5_TARGET_CANVAS_SELECTOR

#endif // BASE_PLATFORM_EMSCRIPTEN

#endif // GRAPHICS_EMSCRIPTEN_H_HEADER_GUARD
