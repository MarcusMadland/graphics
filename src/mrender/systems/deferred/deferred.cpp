#include "mrender/systems/deferred/deferred.hpp"

#include <bgfx/bgfx.h> // @todo Make a wrapper around bgfx tags for tags we want to support

namespace mrender {

Deferred::Deferred(GfxContext* context)
	: RenderSystem("Deferred")
	, mGeometryState(INVALID_HANDLE)
	, mGeometryFramebuffer(INVALID_HANDLE)
	, mLightState(INVALID_HANDLE)
	, mLightFramebuffer(INVALID_HANDLE)
	, mPointLightShader(INVALID_HANDLE)
	, mSpotLightShader(INVALID_HANDLE)
	, mDirectionalLightShader(INVALID_HANDLE)
	, mCombineState(INVALID_HANDLE)
	, mCombineFramebuffer(INVALID_HANDLE)
	, mCombineShader(INVALID_HANDLE)
	, mScreenQuad(INVALID_HANDLE)
{
	context->addSharedBuffer("GDepth", context->createTexture(TextureFormat::D32F, BGFX_TEXTURE_RT));
	context->addSharedBuffer("GDiffuse", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	context->addSharedBuffer("GNormal", context->createTexture(TextureFormat::RGBA32F, BGFX_TEXTURE_RT));
	context->addSharedBuffer("GSpecular", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	context->addSharedBuffer("Light", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	context->addSharedBuffer("Combine", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));

	context->addSystemOption("SkipForwardPass", { false });
}

Deferred::~Deferred()
{
}

bool Deferred::init(GfxContext* context)
{ 
	// Render State
	mGeometryState = context->createRenderState("Color Pass #1 (4 Targets + Depth)", 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_MSAA
		, RenderOrder::DepthAscending);

	mLightState = context->createRenderState("Color Pass #2 (1 Targets)", 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_BLEND_ADD);

	mCombineState = context->createRenderState("Color Pass #3 (1 Targets)", 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_DEPTH_TEST_LESS);

	// Framebuffer
	mGeometryFramebuffer = context->createFramebuffer({
		{ "GDepth", context->getSharedBuffers().at("GDepth") },
		{ "GDiffuse", context->getSharedBuffers().at("GDiffuse") },
		{ "GNormal", context->getSharedBuffers().at("GNormal") },
		{ "GSpecular", context->getSharedBuffers().at("GSpecular") },
		});

	mLightFramebuffer = context->createFramebuffer({
		{ "Light", context->getSharedBuffers().at("Light") },
		});

	mCombineFramebuffer = context->createFramebuffer({
		{ "GDepth", context->getSharedBuffers().at("GDepth") },
		{ "Combine", context->getSharedBuffers().at("Combine") },
		});

	// Light Material
	mPointLightShader = context->createShader(
		"C:/Users/marcu/Dev/mengine/mrender/shaders/build/screen-vert.bin",
		"C:/Users/marcu/Dev/mengine/mrender/shaders/build/deferred_light_point-frag.bin");
	mSpotLightShader = context->createShader(
		"C:/Users/marcu/Dev/mengine/mrender/shaders/build/screen-vert.bin",
		"C:/Users/marcu/Dev/mengine/mrender/shaders/build/deferred_light_spot-frag.bin");
	mDirectionalLightShader = context->createShader(
		"C:/Users/marcu/Dev/mengine/mrender/shaders/build/screen-vert.bin",
		"C:/Users/marcu/Dev/mengine/mrender/shaders/build/deferred_light_directional-frag.bin");
	mCombineShader = context->createShader(
		"C:/Users/marcu/Dev/mengine/mrender/shaders/build/screen-vert.bin",
		"C:/Users/marcu/Dev/mengine/mrender/shaders/build/deferred_combine-frag.bin");

	// Screen quad
	BufferLayout layout =
	{
		{ BufferElement::AttribType::Float, BufferElement::Attrib::Position, 3 },
		{ BufferElement::AttribType::Float, BufferElement::Attrib::TexCoord0, 2 },
	};
	mScreenQuad = context->createGeometry(layout, mQuadVertices.data(), static_cast<uint32_t>(mQuadVertices.size() * sizeof(VertexData)), mQuadIndices);


    return true;
}

void Deferred::render(GfxContext* context)
{
	PROFILE_SCOPE(mName);

	// Devide into two different renderable list. One for deferred rendering and one for forward rendering
	RenderableList deferredRenderables;
	RenderableList forwardRenderables;
	{
		PROFILE_SCOPE("Seperating");
		for (auto& renderable : context->getActiveRenderables())
		{
			if (context->getShaderName(context->getMaterialShader(context->getRenderableMaterial(renderable))) == "deferred_geo")
			{
				deferredRenderables.push_back(renderable);
			}
			else
			{
				forwardRenderables.push_back(renderable);
			}
		}
		for (static bool doOnce = true; doOnce; doOnce = false)
		{
			printf("There is %u renderables being shaded with deferred rendering\n", static_cast<int>(deferredRenderables.size()));
			printf("There is %u renderables being shaded with forward rendering\n", static_cast<int>(forwardRenderables.size()));
		}
	}
	
	{
		PROFILE_SCOPE("Rendering");

		// Set current render pass and clear screen
		context->setActiveRenderState(mGeometryState);
		context->setActiveFramebuffer(mGeometryFramebuffer);
		context->clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);

		// Submit scene
		context->submit(deferredRenderables, context->getActiveCamera());
	}

	{
		PROFILE_SCOPE("Lights");

		// Set current render pass and clear screen
		context->setActiveRenderState(mLightState);
		context->setActiveFramebuffer(mLightFramebuffer);
		context->clear(BGFX_CLEAR_COLOR);

		TextureHandle normalBuffer = context->getSharedBuffers().at("GNormal");
		TextureHandle depthBuffer = context->getSharedBuffers().at("GDepth");
		TextureHandle shadowMap = context->getSharedBuffers().at("ShadowMap");

		for (auto& light : context->getActiveLights())
		{
			LightSettings lightSettings = context->getLightSettings(light);
			CameraSettings cameraSettings = context->getCameraSettings(context->getActiveCamera());

			// Sher correct shader based on type
			ShaderHandle lightShader;
			switch (lightSettings.mType)
			{
			case LightSettings::LightType::Point:
				lightShader = mPointLightShader;
				break;
			case LightSettings::LightType::Spot:
				lightShader = mSpotLightShader;
				break;
			case LightSettings::LightType::Directional:
				lightShader = mDirectionalLightShader;
				break;
			default:
				lightShader = mPointLightShader;
				break;
			}

			// Set shader uniforms (textures)
			context->setTexture(lightShader, "u_gnormal", normalBuffer, 0);
			context->setTexture(lightShader, "u_gdepth", depthBuffer, 1);
			context->setTexture(lightShader, "u_shadowMap", shadowMap, 2);

			// Set shader uniforms (data)
			float positionRange[4] = { lightSettings.mPosition[0], lightSettings.mPosition[1], lightSettings.mPosition[2], lightSettings.mRange };
			context->setUniform(lightShader, "u_lightPositionRange", positionRange);

			float colorIntensity[4] = { lightSettings.mColor[0], lightSettings.mColor[1], lightSettings.mColor[2], lightSettings.mIntensity };
			context->setUniform(lightShader, "u_lightColorIntensity", colorIntensity);

			context->setUniform(lightShader, "u_mtx", context->getCameraViewProj(context->getActiveCamera()));
			if (context->getSharedUniformData().at("u_shadowViewProj").mValue == nullptr) { printf("INVALID SHARED UNIFORM DATA\n"); }
			context->setUniform(lightShader, "u_lightMtx", context->getSharedUniformData().at("u_shadowViewProj").mValue);

			// Submit quad
			context->submit(mScreenQuad, lightShader, INVALID_HANDLE);
		}
	}
	
	{
		PROFILE_SCOPE("Combine");

		context->setActiveRenderState(mCombineState);
		context->setActiveFramebuffer(mCombineFramebuffer);
		context->clear(BGFX_CLEAR_COLOR);
		
		TextureHandle diffuseBuffer = context->getSharedBuffers().at("GDiffuse");
		TextureHandle lightBuffer = context->getSharedBuffers().at("Light");

		context->setTexture(mCombineShader, "u_gdiffuse", diffuseBuffer, 0);
		context->setTexture(mCombineShader, "u_light", lightBuffer, 1);

		// Submit quad
		context->submit(mScreenQuad, mCombineShader, INVALID_HANDLE);
	}

	{
		PROFILE_SCOPE("Forward Pass");
		if (context->getOptionValue<bool>("SkipForwardPass"))
		{
			return;
		}
		context->submit(forwardRenderables, context->getActiveCamera());
	}
}

}   // namespace mrender