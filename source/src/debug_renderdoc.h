/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#ifndef GRAPHICS_RENDERDOC_H_HEADER_GUARD
#define GRAPHICS_RENDERDOC_H_HEADER_GUARD

namespace graphics
{
	void* loadRenderDoc();
	void unloadRenderDoc(void*);
	void renderDocTriggerCapture();

} // namespace graphics

#endif // GRAPHICS_RENDERDOC_H_HEADER_GUARD
