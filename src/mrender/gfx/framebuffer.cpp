#include "mrender/gfx/framebuffer.hpp"
#include "mrender/gfx/texture.hpp"

namespace mrender {

FramebufferImplementation::FramebufferImplementation(RenderContext& context, std::vector<std::string> buffers)
{
	std::vector<bgfx::TextureHandle> fbtextures;
	for (auto& buffer : buffers)
	{
		auto& texture = context.getBuffers().at(buffer);
		auto textureImpl = std::static_pointer_cast<TextureImplementation>(texture);
		fbtextures.push_back(textureImpl->mHandle);
	}
	mHandle = bgfx::createFrameBuffer(static_cast<uint8_t>(fbtextures.size()), fbtextures.data(), false);
}

FramebufferImplementation::~FramebufferImplementation()
{
	if (bgfx::isValid(mHandle))
	{
		bgfx::destroy(mHandle);
	}
}

}	// namespace mrender
