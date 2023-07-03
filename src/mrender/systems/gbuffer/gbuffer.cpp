#include "mrender/systems/gbuffer/gbuffer.hpp"

#include <bgfx/bgfx.h> // @todo Make a wrapper around bgfx tags for tags we want to support

namespace mrender {

GBuffer::GBuffer()
    : RenderSystem("G Buffer"), mState(INVALID_HANDLE), mFramebuffer(INVALID_HANDLE)
{
}

GBuffer::~GBuffer()
{
}

bool GBuffer::init(GfxContext* context)
{ 
	// Render State
	mState = context->createRenderState(0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_MSAA);

	// Framebuffer
	mFramebuffer = context->createFramebuffer(mBuffers);

    return true;
}

void GBuffer::render(GfxContext* context)
{
	PROFILE_SCOPE(mName);

	// Set current render pass id
	context->setActiveRenderState(mState);

	// Clear 
	context->setActiveFramebuffer(mFramebuffer);
	context->clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
		
	// Submit scene
	{
		PROFILE_SCOPE("Render Scene");

		auto renderables = context->getActiveRenderables();
		context->submit(renderables, context->getActiveCamera());
	}
}

BufferList GBuffer::getBuffers(GfxContext* context)
{
	mBuffers.emplace("GDiffuse", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mBuffers.emplace("GNormal", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mBuffers.emplace("GSpecular", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mBuffers.emplace("GPosition", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mBuffers.emplace("GDepth", context->createTexture(TextureFormat::D32F, BGFX_TEXTURE_RT));
	return mBuffers;
}

}   // namespace mrender