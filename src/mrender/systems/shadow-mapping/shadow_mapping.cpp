#include "mrender/systems/shadow-mapping/shadow_mapping.hpp"

#include <bgfx/bgfx.h> // @todo Make a wrapper around bgfx tags for tags we want to support

namespace mrender {

// @todo Make a render option system
static constexpr uint32_t shadowSize = 512;

ShadowMapping::ShadowMapping()
    : RenderSystem("Shadow Mapping")
{
}

ShadowMapping::~ShadowMapping()
{
}

bool ShadowMapping::init(mrender::GfxContext* context)
{ 
	// Shader
	mShader = context->createShader("shadow", "C:/Users/marcu/Dev/mengine/mrender/shaders/shadow");

	// Render State
	mState = context->createRenderState("Shadow Mapping", 0
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_MSAA);

	// Framebuffer
	mFramebuffer = context->createFramebuffer(mBuffers);

	// Camera
	CameraSettings cameraSettings;
	cameraSettings.mProjectionType = CameraSettings::Orthographic;
	cameraSettings.mWidth = 30.0f;
	cameraSettings.mHeight = 30.0f;
	cameraSettings.mPosition[0] = 5.0f;
	cameraSettings.mPosition[1] = 5.0f;
	cameraSettings.mPosition[2] = 2.0f;
	mCamera = context->createCamera(cameraSettings);
    
    return true;
}

void ShadowMapping::render(GfxContext* context)
{
	PROFILE_SCOPE(mName);

	// Set current render pass id
	context->setActiveRenderState(mState);

	// Clear render pass
	context->setActiveFramebuffer(mFramebuffer);
	context->clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, shadowSize, shadowSize);

	// Submit scene
	{
		PROFILE_SCOPE("Render Scene");

		auto renderables = context->getActiveRenderables();
		context->submit(renderables, mShader, mCamera);
	}
}

BufferList ShadowMapping::getBuffers(GfxContext* context)
{
	mBuffers.emplace("ShadowMap", context->createTexture(TextureFormat::D32F, BGFX_TEXTURE_RT, shadowSize, shadowSize));
	return mBuffers;
}

UniformDataList ShadowMapping::getUniformData(GfxContext* context)
{
	UniformDataList uniformDataList;
	uniformDataList.emplace("u_shadowViewProj", UniformData(UniformData::UniformType::Mat4, context->getCameraProjection(mCamera)));
	return uniformDataList;
}

}   // namespace mrender