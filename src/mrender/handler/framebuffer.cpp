#include "mrender/handler/framebuffer.hpp"
#include "mrender/handler/texture.hpp"

namespace mrender {

FramebufferImplementation::FramebufferImplementation(std::vector<std::shared_ptr<Texture>> textures)
{
	std::vector<bgfx::TextureHandle> fbtextures;
	for (auto& texture : textures)
	{
		auto textureImpl = std::static_pointer_cast<TextureImplementation>(texture);
		fbtextures.push_back(textureImpl->mHandle);
	}
	mHandle = bgfx::createFrameBuffer(fbtextures.size(), fbtextures.data(), false);
}

FramebufferImplementation::~FramebufferImplementation()
{
	if (bgfx::isValid(mHandle))
	{
		bgfx::destroy(mHandle);
	}
}

}	// namespace mrender
