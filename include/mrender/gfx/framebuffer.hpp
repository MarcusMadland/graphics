#pragma once

#include "mrender/mrender.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

class FramebufferImplementation : public Framebuffer
{
	friend class RenderContextImplementation;

public:
	FramebufferImplementation(std::vector<std::shared_ptr<Texture>> textures);
	~FramebufferImplementation();

private:
	bgfx::FrameBufferHandle mHandle = BGFX_INVALID_HANDLE;
};

}	// namespace mrender