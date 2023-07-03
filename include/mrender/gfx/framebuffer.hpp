#pragma once

#include "mrender/mrender.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

class FramebufferImplementation : public Framebuffer
{
	friend class GfxContextImplementation;

public:
	FramebufferImplementation(GfxContext* context, BufferList buffers);
	~FramebufferImplementation();

private:
	bgfx::FrameBufferHandle mHandle = BGFX_INVALID_HANDLE;
};

}	// namespace mrender