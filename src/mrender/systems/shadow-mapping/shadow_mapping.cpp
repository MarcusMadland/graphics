#include "mrender/systems/shadow-mapping/shadow_mapping.hpp"

#include <bgfx/bgfx.h> // @todo Make a wrapper around bgfx tags for tags we want to support

namespace mrender {

// @todo Make a render option system
static constexpr bool useShadowSampler = true;
static constexpr uint32_t shadowSize = 512;

ShadowMapping::ShadowMapping()
    : RenderSystem("Shadow Mapping")
{
}

ShadowMapping::~ShadowMapping()
{
}

bool ShadowMapping::init(mrender::RenderContext& context)
{ 
	// Shader
	context.loadShader("shadow", "C:/Users/marcu/Dev/mengine/mrender/shaders/shadow");

	// Render State
	mState = context.createRenderState("Shadow Mapping", 0
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_CULL_CCW
		| BGFX_STATE_MSAA);

	// Framebuffer
	mFramebuffer = context.createFramebuffer({ "ShadowMap" });

	// Camera
	CameraSettings cameraSettings;
	cameraSettings.mProjectionType = ProjectionType::Orthographic;
	cameraSettings.mWidth = 30.0f;
	cameraSettings.mHeight = 30.0f;
	cameraSettings.mPosition[0] = 5.0f;
	cameraSettings.mPosition[1] = 5.0f;
	cameraSettings.mPosition[2] = 2.0f;
	mCamera = context.createCamera(cameraSettings);
    
    return true;
}

void ShadowMapping::render(RenderContext& context)
{
	// Set current render pass id
	context.setRenderState(mState);

	// Clear render pass
	context.writeToBuffers(mFramebuffer);
	context.clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, shadowSize, shadowSize);

	// Submit scene
	auto renderables = context.getRenderables();
	context.submit(renderables, mCamera);
}

std::unordered_map<std::string, std::shared_ptr<Texture>> ShadowMapping::getBuffers(RenderContext& context)
{
	std::unordered_map<std::string, std::shared_ptr<Texture>> buffers;
	buffers.emplace("ShadowMap", context.createTexture(TextureFormat::D16, BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL, shadowSize, shadowSize));
	return buffers;
}

}   // namespace mrender