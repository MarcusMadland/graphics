#include "mrender/gfx/framebuffer.hpp"
#include "mrender/gfx/texture.hpp"
#include "mrender/gfx/render_context.hpp"

namespace mrender {

FramebufferImplementation::FramebufferImplementation(GfxContext* context, BufferList buffers)
{
	auto contextImpl = static_cast<GfxContextImplementation*>(context);

	std::vector<bgfx::TextureHandle> fbtextures;
	for (auto& buffer : buffers)
	{
		//auto textureImpl = STATIC_IMPL_CAST(Texture, contextImpl->getTextureData(buffer.second));
		auto textureImpl = STATIC_IMPL_CAST(Texture, contextImpl->getTextureData(buffer.second));
		if (bgfx::isValid(textureImpl->mHandle))
		{
			fbtextures.push_back(textureImpl->mHandle);
		}
		else
			printf("Invalid texture data from TextureHandle\n");
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
