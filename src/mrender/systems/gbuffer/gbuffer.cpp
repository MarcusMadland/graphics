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
	mGeometryState = context->createRenderState("Geometry Buffer", 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_MSAA);

	mLightState = context->createRenderState("Light Buffer", 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A);

	mCombineState = context->createRenderState("Combine Buffers", 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A);

	// Framebuffer
	mGeometryFramebuffer = context->createFramebuffer(mGeometryBuffers);
	mLightFramebuffer = context->createFramebuffer(mLightBuffers);
	mCombineFramebuffer = context->createFramebuffer(mCombineBuffers);

	// Light Material
	mLightShader = context->createShader("deferred_light", "C:/Users/marcu/Dev/mengine/mrender/shaders/deferred_light");
	mCombineShader = context->createShader("deferred_combine", "C:/Users/marcu/Dev/mengine/mrender/shaders/deferred_combine");

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

	// Sorting back to front
	RenderableList sortedRenderables;
	{
		PROFILE_SCOPE("FTB Sorting");

		// Calculate distances from the camera for each renderable
		auto renderables = context->getActiveRenderables();
		std::vector<std::pair<RenderableHandle, float>> renderableDistances;
		renderableDistances.reserve(renderables.size());

		const CameraSettings cameraSettings = context->getCameraSettings(context->getActiveCamera());
		float cameraPos[3] = { cameraSettings.mPosition[0], cameraSettings.mPosition[1], cameraSettings.mPosition[2] };

		for (auto& renderable : renderables)
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
			sortedRenderables.push_back(renderable.first);
		}
	}
	
	{
		PROFILE_SCOPE("Geometry Buffer");

		// Set current render pass and clear screen
		context->setActiveRenderState(mGeometryState);
		context->setActiveFramebuffer(mGeometryFramebuffer);
		context->clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);

		// Submit scene
		context->submit(sortedRenderables, context->getActiveCamera());
	}
	
	{
		PROFILE_SCOPE("Light Buffer");

		// Set current render pass and clear screen
		context->setActiveRenderState(mLightState);
		context->setActiveFramebuffer(mLightFramebuffer);
		context->clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);

		// Set shader uniforms
		TextureHandle diffuseBuffer = context->getSharedBuffers().at("GDiffuse");
		context->setTexture(mLightShader, "u_gdiffuse", diffuseBuffer, 0);

		TextureHandle normalBuffer = context->getSharedBuffers().at("GNormal");
		context->setTexture(mLightShader, "u_gnormal", normalBuffer, 1);

		TextureHandle specularBuffer = context->getSharedBuffers().at("GSpecular");
		context->setTexture(mLightShader, "u_gspecular", specularBuffer, 2);

		TextureHandle shadowMap = context->getSharedBuffers().at("ShadowMap");
		context->setTexture(mLightShader, "u_shadowMap", shadowMap,3);

		static float sunLightDirection[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		context->setUniform(mLightShader, "u_lightDir", &sunLightDirection);

		static float sunLightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		context->setUniform(mLightShader, "u_lightColor", &sunLightColor);

		context->setUniform(mLightShader, "u_shadowViewProj", context->getSharedUniformData().at("u_shadowViewProj").mValue);
		

		// Submit quad
		context->submit(mScreenQuad, mLightShader, INVALID_HANDLE);
	}
	
	{
		PROFILE_SCOPE("Combine Buffers");

		// Set current render pass and clear screen
		context->setActiveRenderState(mCombineState);
		context->setActiveFramebuffer(mCombineFramebuffer);
		context->clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);

		// Set shader uniforms
		TextureHandle diffuseBuffer = context->getSharedBuffers().at("GDiffuse");
		context->setTexture(mCombineShader, "u_gdiffuse", diffuseBuffer, 0);

		TextureHandle lightBuffer = context->getSharedBuffers().at("Light");
		context->setTexture(mCombineShader, "u_light", lightBuffer, 1);

		// Submit quad
		context->submit(mScreenQuad, mCombineShader, INVALID_HANDLE);
	}
}

BufferList GBuffer::getBuffers(GfxContext* context)
{
	mGeometryBuffers.emplace("GDiffuse", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mGeometryBuffers.emplace("GNormal", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mGeometryBuffers.emplace("GSpecular", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mGeometryBuffers.emplace("GDepth", context->createTexture(TextureFormat::D32F, BGFX_TEXTURE_RT));
	mLightBuffers.emplace("Light", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mCombineBuffers.emplace("Combine", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));

	BufferList buffers;
	buffers.insert(mGeometryBuffers.begin(), mGeometryBuffers.end());
	buffers.insert(mLightBuffers.begin(), mLightBuffers.end());
	buffers.insert(mCombineBuffers.begin(), mCombineBuffers.end());
	return buffers;
}

UniformDataList GBuffer::getUniformData(GfxContext* context)
{
	return UniformDataList();
}

}   // namespace mrender