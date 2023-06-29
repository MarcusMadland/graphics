#include "mrender/systems/gbuffer/gbuffer.hpp"

#include <bgfx/bgfx.h> // @todo Make a wrapper around bgfx tags for tags we want to support

namespace mrender {

GBuffer::GBuffer()
    : RenderSystem("G Buffer")
{
}

GBuffer::~GBuffer()
{
}

bool GBuffer::init(mrender::RenderContext& context)
{ 
	// Render State
	mState = context.createRenderState("G Buffers", 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_MSAA);

	// Framebuffer
	mFramebuffer = context.createFramebuffer({ "GDiffuse", "GNormal", "GSpecular", "GPosition", "GDepth" });

    return true;
}

void GBuffer::render(RenderContext& context)
{
	PROFILE_SCOPE(mName);

	// Set current render pass id
	context.setRenderState(mState);

	// Clear 
	{
		context.writeToBuffers(mFramebuffer);
		context.clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
		
		// Submit scene
		{
			PROFILE_SCOPE("Render Scene");

			auto renderables = context.getRenderables();
			context.submit(renderables, context.getCamera());
		}
	}
}

std::unordered_map<std::string, std::shared_ptr<Texture>> GBuffer::getBuffers(RenderContext& context)
{
	std::unordered_map<std::string, std::shared_ptr<Texture>> buffers;
	buffers.emplace("GDiffuse", context.createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	buffers.emplace("GNormal", context.createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	buffers.emplace("GSpecular", context.createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	buffers.emplace("GPosition", context.createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	buffers.emplace("GDepth", context.createTexture(TextureFormat::D32F, BGFX_TEXTURE_RT));
	return buffers;
}

}   // namespace mrender