#pragma once

#include "mrender/mrender.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

class FramebufferImplementation : public Framebuffer
{
	friend class RenderContextImplementation;

public:
	FramebufferImplementation(RenderContext& context, std::vector<std::string> buffers);
	~FramebufferImplementation();

private:
	bgfx::FrameBufferHandle mHandle = BGFX_INVALID_HANDLE;
};

}	// namespace mrender