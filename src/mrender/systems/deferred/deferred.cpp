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

	// Framebuffer
	mGeometryFramebuffer = context->createFramebuffer(mGeometryBuffers);
	//mLightBuffers.emplace("LightDepth", mGeometryBuffers.at("GDepth"));
	mLightFramebuffer = context->createFramebuffer(mLightBuffers);

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
	
	// Sorting back to front
	RenderableList sortedDeferredRenderables;
	{
		PROFILE_SCOPE("FTB Sorting");

		// Calculate distances from the camera for each renderable
		std::vector<std::pair<RenderableHandle, float>> renderableDistances;
		renderableDistances.reserve(deferredRenderables.size());

		const CameraSettings cameraSettings = context->getCameraSettings(context->getActiveCamera());
		float cameraPos[3] = { cameraSettings.mPosition[0], cameraSettings.mPosition[1], cameraSettings.mPosition[2] };

		for (auto& renderable : deferredRenderables)
		{
			float* renderableMatrix = context->getRenderableTransform(renderable);
			float renderablePos[3] = { renderableMatrix[12], renderableMatrix[13], renderableMatrix[14] };

			float dx = renderablePos[0] - cameraPos[0];
			float dy = renderablePos[1] - cameraPos[1];
			float dz = renderablePos[2] - cameraPos[2];
			float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
			renderableDistances.emplace_back(renderable, distance);
		}

		// Sort renderables based on distance in descending order (farthest to nearest)
		std::sort(renderableDistances.begin(), renderableDistances.end(),
			[](const std::pair<RenderableHandle, float>& a, const std::pair<RenderableHandle, float>& b)
			{
				return a.second < b.second;
			});

		// Make vector from map
		for (auto& renderable : renderableDistances)
		{
			sortedDeferredRenderables.push_back(renderable.first);
		}
	}
	
	{
		PROFILE_SCOPE("Rendering");

		// Set current render pass and clear screen
		context->setActiveRenderState(mGeometryState);
		context->setActiveFramebuffer(mGeometryFramebuffer);
		context->clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);

		// Submit scene
		context->submit(sortedDeferredRenderables, context->getActiveCamera());
	}
	
	{
		PROFILE_SCOPE("Lights");

		// Set current render pass and clear screen
		context->setActiveRenderState(mLightState);
		context->setActiveFramebuffer(mLightFramebuffer);
		context->clear(BGFX_CLEAR_COLOR);

		TextureHandle positionBuffer = context->getSharedBuffers().at("GPosition");
		TextureHandle normalBuffer = context->getSharedBuffers().at("GNormal");
		TextureHandle depthBuffer = context->getSharedBuffers().at("GDepth");

		for (auto& light : context->getActiveLights())
		{
			LightSettings lightSettings = context->getLightSettings(light);
			CameraSettings cameraSettings = context->getCameraSettings(context->getActiveCamera());

			// Sher correct shader based on type
			ShaderHandle lightShader;
			switch (lightSettings.mLightType)
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
			context->setTexture(lightShader, "u_gposition", positionBuffer, 2);

			// Set shader uniforms (data)
			float positionRange[4] = { lightSettings.mPosition[0], lightSettings.mPosition[1], lightSettings.mPosition[2], lightSettings.mRange };
			context->setUniform(lightShader, "u_lightPositionRange", positionRange);

			float colorIntensity[4] = { lightSettings.mColor[0], lightSettings.mColor[1], lightSettings.mColor[2], lightSettings.mIntensity };
			context->setUniform(lightShader, "u_lightColorIntensity", colorIntensity);

			// Submit quad
			context->submit(mScreenQuad, lightShader, INVALID_HANDLE);
		}
	}

	{
		PROFILE_SCOPE("Forward Pass");

		context->submit(forwardRenderables, context->getActiveCamera());
	}
}

BufferList Deferred::getBuffers(GfxContext* context)
{
	mGeometryBuffers.emplace("GDepth", context->createTexture(TextureFormat::D24S8, BGFX_TEXTURE_RT));
	mGeometryBuffers.emplace("GDiffuse", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mGeometryBuffers.emplace("GNormal", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mGeometryBuffers.emplace("GSpecular", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mGeometryBuffers.emplace("GPosition", context->createTexture(TextureFormat::RGBA32F, BGFX_TEXTURE_RT));
	mLightBuffers.emplace("Light", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));

	BufferList buffers;
	buffers.insert(mGeometryBuffers.begin(), mGeometryBuffers.end());
	buffers.insert(mLightBuffers.begin(), mLightBuffers.end());
	return buffers;
}

UniformDataList Deferred::getUniformData(GfxContext* context)
{
	return UniformDataList();
}

}   // namespace mrender