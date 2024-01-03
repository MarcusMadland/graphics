/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#include "graphics_p.h"

namespace graphics { namespace agc
{
	RendererContextI* rendererCreate(const Init& _init)
	{
		BASE_UNUSED(_init);
		return NULL;
	}

	void rendererDestroy()
	{
	}

} /* namespace agc */ } // namespace graphics
