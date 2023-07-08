#include "mrender/systems/gbuffer/gbuffer.hpp"

#include <bgfx/bgfx.h> // @todo Make a wrapper around bgfx tags for tags we want to support

namespace mrender {

GBuffer::GBuffer()
	: RenderSystem("G Buffer")
	, mGeometryState(INVALID_HANDLE)
	, mGeometryFramebuffer(INVALID_HANDLE)
	, mLightState(INVALID_HANDLE)
	, mLightFramebuffer(INVALID_HANDLE)
{
}

GBuffer::~GBuffer()
{
}

bool GBuffer::init(GfxContext* context)
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
		| BGFX_STATE_DEPTH_TEST_LEQUAL);

	// Framebuffer
	mGeometryFramebuffer = context->createFramebuffer(mGeometryBuffers);
	mLightBuffers.emplace("LightDepth", mGeometryBuffers.at("GDepth"));
	mLightFramebuffer = context->createFramebuffer(mLightBuffers);

	// Light Material
	mLightShader = context->createShader("deferred_light", "C:/Users/marcu/Dev/mengine/mrender/shaders/deferred_light");

	// Screen quad
	BufferLayout layout =
	{
		{ BufferElement::AttribType::Float, BufferElement::Attrib::Position, 3 },
		{ BufferElement::AttribType::Float, BufferElement::Attrib::TexCoord0, 2 },
	};
	mScreenQuad = context->createGeometry(layout, mQuadVertices.data(), static_cast<uint32_t>(mQuadVertices.size() * sizeof(VertexData)), mQuadIndices);


    return true;
}

void GBuffer::render(GfxContext* context)
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
			printf("There is %u renderables being shaded with deferred rendering\n", static_cast<uint64_t>(deferredRenderables.size()));
			printf("There is %u renderables being shaded with forward rendering\n", static_cast<uint64_t>(forwardRenderables.size()));
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
		PROFILE_SCOPE("Deferred Rendering");

		// Set current render pass and clear screen
		context->setActiveRenderState(mGeometryState);
		context->setActiveFramebuffer(mGeometryFramebuffer);
		context->clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);

		// Submit scene
		context->submit(sortedDeferredRenderables, context->getActiveCamera());
	}
	
	{
		PROFILE_SCOPE("Deferred Light Calculations");

		// Set current render pass and clear screen
		context->setActiveRenderState(mLightState);
		context->setActiveFramebuffer(mLightFramebuffer);
		context->clear(BGFX_CLEAR_COLOR);

		// Set shader uniforms
		TextureHandle diffuseBuffer = context->getSharedBuffers().at("GDiffuse");
		context->setTexture(mLightShader, "u_gdiffuse", diffuseBuffer, 0);

		TextureHandle normalBuffer = context->getSharedBuffers().at("GNormal");
		context->setTexture(mLightShader, "u_gnormal", normalBuffer, 1);

		TextureHandle specularBuffer = context->getSharedBuffers().at("GSpecular");
		context->setTexture(mLightShader, "u_gspecular", specularBuffer, 2);

		TextureHandle positionBuffer = context->getSharedBuffers().at("GPosition");
		context->setTexture(mLightShader, "u_gposition", positionBuffer, 3);

		TextureHandle shadowMap = context->getSharedBuffers().at("ShadowMap");
		context->setTexture(mLightShader, "u_shadowMap", shadowMap,4);

		static float sunLightDirection[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		context->setUniform(mLightShader, "u_lightDir", &sunLightDirection);

		static float sunLightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		context->setUniform(mLightShader, "u_lightColor", &sunLightColor);

		struct LightData
		{
			float position[4];
		};
		static float lightPositions[4][4]
		{
			{ 3.0f, 0.0f, 3.0f, 0.0f }, 
			{ -3.0f, 0.0f, 3.0f, 0.0f },
			{ 3.0f, 0.0f, -3.0f, 0.0f },
			{ -3.0f, 0.0f, -3.0f, 0.0f },
		};
		static float lightColors[4][4]
		{
			{ 1.0f, 1.0f , 1.0f, 1.0f },
			{ 1.0f, 1.0f , 1.0f, 1.0f },
			{ 1.0f, 1.0f , 1.0f, 1.0f },
			{ 1.0f, 1.0f , 1.0f, 1.0f },
		};

		context->setUniform(mLightShader, "u_lightPositions", &lightPositions[0][0]);
		context->setUniform(mLightShader, "u_lightColors", &lightColors[0][0]);
		
		context->setUniform(mLightShader, "u_viewPos", context->getCameraSettings(context->getActiveCamera()).mPosition);

		context->setUniform(mLightShader, "u_shadowViewProj", context->getSharedUniformData().at("u_shadowViewProj").mValue);
		

		// Submit quad
		context->submit(mScreenQuad, mLightShader, INVALID_HANDLE);
	}

	{
		PROFILE_SCOPE("Forward Rendering");

		context->submit(forwardRenderables, context->getActiveCamera());
	}
}

BufferList GBuffer::getBuffers(GfxContext* context)
{
	mGeometryBuffers.emplace("GDepth", context->createTexture(TextureFormat::D24S8, BGFX_TEXTURE_RT));
	mGeometryBuffers.emplace("GDiffuse", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mGeometryBuffers.emplace("GNormal", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mGeometryBuffers.emplace("GSpecular", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mGeometryBuffers.emplace("GPosition", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mLightBuffers.emplace("Light", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));

	BufferList buffers;
	buffers.insert(mGeometryBuffers.begin(), mGeometryBuffers.end());
	buffers.insert(mLightBuffers.begin(), mLightBuffers.end());
	return buffers;
}

UniformDataList GBuffer::getUniformData(GfxContext* context)
{
	return UniformDataList();
}

}   // namespace mrender