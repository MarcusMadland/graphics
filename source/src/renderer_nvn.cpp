/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#include "graphics_p.h"

namespace graphics
{

#define GRAPHICS_DECLARE_EMBEDDED_SHADER(_name)                                           \
	extern const uint8_t* BASE_CONCATENATE(_name, _nvn);                                \
	extern const uint32_t BASE_CONCATENATE(_name, _nvn_size);                           \
	static const uint8_t  BASE_CONCATENATE(_name, _int_nvn)[] = { 0 };                  \
	const uint8_t* BASE_CONCATENATE(_name, _nvn) = &BASE_CONCATENATE(_name, _int_nvn)[0]; \
	const uint32_t BASE_CONCATENATE(_name, _nvn_size) = 1

GRAPHICS_DECLARE_EMBEDDED_SHADER(vs_debugfont);
GRAPHICS_DECLARE_EMBEDDED_SHADER(fs_debugfont);
GRAPHICS_DECLARE_EMBEDDED_SHADER(vs_clear);
GRAPHICS_DECLARE_EMBEDDED_SHADER(fs_clear0);
GRAPHICS_DECLARE_EMBEDDED_SHADER(fs_clear1);
GRAPHICS_DECLARE_EMBEDDED_SHADER(fs_clear2);
GRAPHICS_DECLARE_EMBEDDED_SHADER(fs_clear3);
GRAPHICS_DECLARE_EMBEDDED_SHADER(fs_clear4);
GRAPHICS_DECLARE_EMBEDDED_SHADER(fs_clear5);
GRAPHICS_DECLARE_EMBEDDED_SHADER(fs_clear6);
GRAPHICS_DECLARE_EMBEDDED_SHADER(fs_clear7);

#undef GRAPHICS_DECLARE_EMBEDDED_SHADER

} // namespace graphics

namespace graphics { namespace nvn
{
	RendererContextI* rendererCreate(const Init& _init)
	{
		BASE_UNUSED(_init);
		return NULL;
	}

	void rendererDestroy()
	{
	}

} /* namespace nvn */ } // namespace graphics
