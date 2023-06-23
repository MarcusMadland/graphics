#include "mrender/systems/gbuffer/gbuffer.hpp"
#include "mrender/core/file_ops.hpp"


#include "mrender/handler/shader.hpp" // @temp
#include "mrender/handler/render_context.hpp"
#include "mrender/handler/texture.hpp"
#include "mrender/handler/framebuffer.hpp"

#include <bgfx/bgfx.h>
#include <bx/math.h>

#define RENDER_GEOMETRY_PASS_ID 1

namespace mrender {

// @todo make options
static constexpr bool useShadowSampler = true;
static constexpr uint32_t shadowSize = 512;

GBuffer::GBuffer()
    : RenderSystem("G Buffer")
{
}

GBuffer::~GBuffer()
{
}

bool GBuffer::init(mrender::RenderContext& context)
{ 
	// Framebuffer
	mFramebuffer = context.createFramebuffer({ "GBufferColor", "GBufferDepth" });
	if (mFramebuffer == nullptr) { printf("Failed to create framebuffer"); }

	// Render State
	mState = context.createRenderState(0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_MSAA);
	
    return true;
}

void GBuffer::render(RenderContext& context)
{
	// Set current render pass id
	context.setRenderState(mState);

	// Clear render pass
	context.writeToBuffers(mFramebuffer);
	context.clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);

	// Submit scene
	context.submit(context.getRenderables(), context.getCamera());
}

std::unordered_map<std::string, std::shared_ptr<Texture>> GBuffer::getBuffers(RenderContext& context)
{
	std::unordered_map<std::string, std::shared_ptr<Texture>> buffers;
	buffers.emplace("GBufferColor", context.createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	buffers.emplace("GBufferDepth", context.createTexture(TextureFormat::D32F, BGFX_TEXTURE_RT));
	return buffers;
}

}   // namespace mrender