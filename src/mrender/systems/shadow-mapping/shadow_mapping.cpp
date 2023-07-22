#include "mrender/systems/shadow-mapping/shadow_mapping.hpp"

#include <bgfx/bgfx.h> // @todo Make a wrapper around bgfx tags for tags we want to support
#include <bx/math.h>

namespace mrender {

ShadowMapping::ShadowMapping(GfxContext* context)
    : RenderSystem("Shadow Mapping")
	, mShader(INVALID_HANDLE)
	, mState(INVALID_HANDLE)
	, mFramebuffer(INVALID_HANDLE)
	, mCamera(INVALID_HANDLE)
{
	Option shadowMapSizeOption;
	shadowMapSizeOption.mValue = 2048;
	shadowMapSizeOption.mMin = 16;
	shadowMapSizeOption.mMax = 2048;
	context->addSystemOption("ShadowMapSize", shadowMapSizeOption);

	Option directionalLightPositionOption;
	directionalLightPositionOption.mValue = std::array<float, 3>{5.0f, 5.0f, 2.0f};
	directionalLightPositionOption.mMin = -15.0f;
	directionalLightPositionOption.mMax = 15.0f;
	context->addSystemOption("DirectionalLightPosition", directionalLightPositionOption);

	uint16_t shadowMapSize = context->getOptionValue<int>("ShadowMapSize");
	std::array<float, 3> directionalLightPosition = context->getOptionValue<std::array<float, 3>>("DirectionalLightPosition");

	bgfx::setPaletteColor(1, 0xFFFFFFFF); // @todo Make abstraction for this
	context->addSharedBuffer("ShadowMap", context->createTexture(TextureFormat::D32F, BGFX_TEXTURE_RT | BGFX_SAMPLER_U_BORDER | BGFX_SAMPLER_V_BORDER | BGFX_SAMPLER_BORDER_COLOR(1), shadowMapSize, shadowMapSize));

	CameraSettings cameraSettings;
	cameraSettings.mProjectionType = CameraSettings::Orthographic;
	cameraSettings.mWidth = 30.0f;
	cameraSettings.mHeight = 30.0f;
	cameraSettings.mClipNear = 0.01f;
	cameraSettings.mClipFar = 100.0f;
	cameraSettings.mPosition[0] = directionalLightPosition[0];
	cameraSettings.mPosition[1] = directionalLightPosition[1];
	cameraSettings.mPosition[2] = directionalLightPosition[2];
	cameraSettings.mLookAt[2] = 1.0f;
	mCamera = context->createCamera(cameraSettings);

	context->addSharedUniformData("u_shadowViewProj", UniformData(UniformData::UniformType::Mat4, context->getCameraViewProj(mCamera)));
}

ShadowMapping::~ShadowMapping()
{
}

bool ShadowMapping::init(mrender::GfxContext* context)
{ 
	// Shader
	mShader = context->createShader(
		"C:/Users/marcu/Dev/mengine/mrender/shaders/build/shadow-vert.bin",
		"C:/Users/marcu/Dev/mengine/mrender/shaders/build/shadow-frag.bin");

	// Render State
	mState = context->createRenderState("Shadow Mapping", 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_CULL_CW
		, RenderOrder::DepthAscending
		);

	// Framebuffer
	mFramebuffer = context->createFramebuffer({
		{ "Light", context->getSharedBuffers().at("ShadowMap") },
		});
    
    return true;
}

void ShadowMapping::render(GfxContext* context)
{
	PROFILE_SCOPE(mName);

	uint16_t shadowMapSize = context->getOptionValue<int>("ShadowMapSize");
	std::array<float, 3> directionalLightPosition = context->getOptionValue<std::array<float, 3>>("DirectionalLightPosition");

	// Camera
	CameraSettings cameraSettings = context->getCameraSettings(mCamera);
	cameraSettings.mPosition[0] = directionalLightPosition[0];
	cameraSettings.mPosition[1] = directionalLightPosition[1];
	cameraSettings.mPosition[2] = directionalLightPosition[2];
	context->setCameraSettings(mCamera, cameraSettings);

	// Set current render pass id
	context->setActiveRenderState(mState);

	// Clear render pass
	context->setActiveFramebuffer(mFramebuffer);
	context->clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, shadowMapSize, shadowMapSize);

	// Submit scene
	{
		PROFILE_SCOPE("Render Scene");

		auto renderables = context->getActiveRenderables();
		context->submit(renderables, mShader, mCamera);
	}
}

}   // namespace mrender