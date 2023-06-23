#pragma once

#include "mrender/mrender.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

class FramebufferImplementation : public Framebuffer
{
	friend class GBuffer;
	friend class ShadowMapping;

public:
	FramebufferImplementation(std::vector<std::shared_ptr<Texture>> textures);
	~FramebufferImplementation();

private:
	bgfx::FrameBufferHandle mHandle;
};

}	// namespace mrender