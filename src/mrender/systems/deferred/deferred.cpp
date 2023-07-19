#include "mrender/systems/deferred/deferred.hpp"

#include <bgfx/bgfx.h> // @todo Make a wrapper around bgfx tags for tags we want to support

namespace mrender {

Deferred::Deferred()
	: RenderSystem("Deferred")
	, mGeometryState(INVALID_HANDLE)
	, mGeometryFramebuffer(INVALID_HANDLE)
	, mLightState(INVALID_HANDLE)
	, mLightFramebuffer(INVALID_HANDLE)
{
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
		| BGFX_STATE_MSAA);

	mLightState = context->createRenderState("Color Pass #2 (1 Targets)", 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_BLEND_ADD);

	mCombineState = context->createRenderState("Color Pass #3 (1 Targets)", 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_DEPTH_TEST_LESS);

	// Framebuffer
	mGeometryFramebuffer = context->createFramebuffer(mGeometryBuffers);
	mCombineBuffers.emplace("CombineDepth", mGeometryBuffers.at("GDepth"));
	mLightFramebuffer = context->createFramebuffer(mLightBuffers);
	mCombineFramebuffer = context->createFramebuffer(mCombineBuffers);

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

	RenderableList deferredRenderables;
	RenderableList forwardRenderables;
	const CameraSettings& cameraSettings = context->getCameraSettings(context->getActiveCamera());

	{
		PROFILE_SCOPE("Seperating");

		for (const auto& renderable : context->getActiveRenderables())
		{
			const ShaderHandle& shader = context->getMaterialShader(context->getRenderableMaterial(renderable));
			
			if (context->getShaderName(shader) == "deferred_geo")
			{
				deferredRenderables.push_back(renderable);
			}
			else
			{
				forwardRenderables.push_back(renderable);
			}
		}

		// @todo This should use some sort of shared stats function similar to getBuffers(). getStats()
		for (static bool doOnce = true; doOnce; doOnce = false)
		{
			printf("There is %u renderables being shaded with deferred rendering\n", static_cast<int>(deferredRenderables.size()));
			printf("There is %u renderables being shaded with forward rendering\n", static_cast<int>(forwardRenderables.size()));
		}
	}
	
	{
		PROFILE_SCOPE("Sorting (ftb)");

		std::vector<std::pair<RenderableHandle, float>> renderableDistances;
		renderableDistances.reserve(deferredRenderables.size());

		const float cameraPos[3] = { cameraSettings.mPosition[0], cameraSettings.mPosition[1], cameraSettings.mPosition[2] };

		for (const auto& renderable : deferredRenderables)
		{
			const float* model = context->getRenderableTransform(renderable);
			const float position[3] = { model[12], model[13], model[14] };
			const float dx = position[0] - cameraPos[0];
			const float dy = position[1] - cameraPos[1];
			const float dz = position[2] - cameraPos[2];
			const float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

			renderableDistances.emplace_back(renderable, distance);
		}

		std::sort(renderableDistances.begin(), renderableDistances.end(),
			[](const std::pair<RenderableHandle, float>& a, const std::pair<RenderableHandle, float>& b)
			{
				return a.second < b.second;
			});

		deferredRenderables.clear();
		for (auto& renderable : renderableDistances)
		{
			deferredRenderables.push_back(renderable.first);
		}
	}
	
	{
		PROFILE_SCOPE("Deferred Render");

		context->setActiveRenderState(mGeometryState);
		context->setActiveFramebuffer(mGeometryFramebuffer);
		context->clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);

		context->submit(deferredRenderables, context->getActiveCamera());
	}

	{
		PROFILE_SCOPE("Lights");

		context->setActiveRenderState(mLightState);
		context->setActiveFramebuffer(mLightFramebuffer);
		context->clear(BGFX_CLEAR_COLOR);

		const TextureHandle normalBuffer = context->getSharedBuffers().at("GNormal");
		const TextureHandle depthBuffer = context->getSharedBuffers().at("GDepth");

		for (const auto& light : context->getActiveLights())
		{
			const LightSettings& lightSettings = context->getLightSettings(light);
			
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
				lightShader = INVALID_HANDLE;
				break;
			}

			float positionRange[4] = { lightSettings.mPosition[0], lightSettings.mPosition[1], lightSettings.mPosition[2], lightSettings.mRange };
			float colorIntensity[4] = { lightSettings.mColor[0], lightSettings.mColor[1], lightSettings.mColor[2], lightSettings.mIntensity };

			context->setTexture(lightShader, "u_gnormal", normalBuffer, 0);
			context->setTexture(lightShader, "u_gdepth", depthBuffer, 1);

			context->setUniform(lightShader, "u_lightPositionRange", positionRange);
			context->setUniform(lightShader, "u_lightColorIntensity", colorIntensity);
			context->setUniform(lightShader, "u_mtx", context->getCameraViewProj(context->getActiveCamera()));

			context->submit(mScreenQuad, lightShader, INVALID_HANDLE);
		}
	}
	
	{
		PROFILE_SCOPE("Combine");

		context->setActiveRenderState(mCombineState);
		context->setActiveFramebuffer(mCombineFramebuffer);
		context->clear(BGFX_CLEAR_COLOR);
		
		const TextureHandle diffuseBuffer = context->getSharedBuffers().at("GDiffuse");
		const TextureHandle lightBuffer = context->getSharedBuffers().at("Light");

		context->setTexture(mCombineShader, "u_gdiffuse", diffuseBuffer, 0);
		context->setTexture(mCombineShader, "u_light", lightBuffer, 1);

		context->submit(mScreenQuad, mCombineShader, INVALID_HANDLE);
	}

	{
		PROFILE_SCOPE("Forward Render");

		context->submit(forwardRenderables, context->getActiveCamera());
	}
}

BufferList Deferred::getBuffers(GfxContext* context)
{
	mGeometryBuffers.emplace("GDepth", context->createTexture(TextureFormat::D32F, BGFX_TEXTURE_RT));
	mGeometryBuffers.emplace("GDiffuse", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mGeometryBuffers.emplace("GNormal", context->createTexture(TextureFormat::RGBA32F, BGFX_TEXTURE_RT));
	mGeometryBuffers.emplace("GSpecular", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mLightBuffers.emplace("Light", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mCombineBuffers.emplace("Combine", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));

	BufferList buffers;
	buffers.insert(mGeometryBuffers.begin(), mGeometryBuffers.end());
	buffers.insert(mLightBuffers.begin(), mLightBuffers.end());
	buffers.insert(mCombineBuffers.begin(), mCombineBuffers.end());
	return buffers;
}

UniformDataList Deferred::getUniformData(GfxContext* context)
{
	return UniformDataList();
}

}   // namespace mrender