#include "mrender/mrender.hpp"

#include "mrender/gfx/shader.hpp"
#include "mrender/gfx/render_context.hpp"
#include "mrender/gfx/camera.hpp"
#include "mrender/gfx/geometry.hpp"
#include "mrender/gfx/texture.hpp"
#include "mrender/gfx/framebuffer.hpp"
#include "mrender/gfx/renderable.hpp"
#include "mrender/gfx/render_state.hpp"
#include "mrender/gfx/material.hpp"

#include <bx/math.h>

namespace mrender {

RenderSystem::RenderSystem(const std::string_view& name)
	: mName(name)
{}

std::shared_ptr<Shader> RenderContext::createShader()
{
	return std::make_shared<ShaderImplementation>();
}

std::shared_ptr<RenderState> RenderContext::createRenderState(std::string name, uint64_t flags)
{
	return std::make_shared<RenderStateImplementation>(*this, name, flags);
}

std::shared_ptr<Framebuffer> RenderContext::createFramebuffer(std::vector<std::string> buffers)
{
	return std::make_shared<FramebufferImplementation>(*this, buffers);
}

std::shared_ptr<Texture> RenderContext::createTexture(TextureFormat format, uint64_t textureFlags, uint16_t width, uint16_t height)
{
	return std::make_shared<TextureImplementation>(format, textureFlags, width, height);
}

std::shared_ptr<Texture> RenderContext::createTexture(const uint8_t* data, TextureFormat format, uint64_t textureFlags, uint16_t width, uint16_t height, uint16_t channels)
{
	return std::make_shared<TextureImplementation>(data, format, textureFlags, width, height, channels);
}

std::shared_ptr<Camera> RenderContext::createCamera(const CameraSettings& settings)
{
	return std::make_shared<CameraImplementation>(settings);
}

std::shared_ptr<Geometry> RenderContext::createGeometry(const BufferLayout& layout, void* vertexData, uint32_t vertexSize, std::vector<uint16_t> indices)
{
	return std::make_shared<GeometryImplementation>(layout, vertexData, vertexSize, indices);
}

std::shared_ptr<Renderable> RenderContext::createRenderable(std::shared_ptr<Geometry> geometry, std::shared_ptr<Material> material)
{
	return std::make_shared<RenderableImplementation>(std::move(geometry), material);
}

std::shared_ptr<Material> RenderContext::createMaterial(const std::string& shaderName)
{
	return std::make_shared<MaterialImplementation>(*this, shaderName);
}

std::shared_ptr<RenderContext> createRenderContext(const RenderSettings& settings)
{
	return std::make_shared<RenderContextImplementation>(settings);
}

}	// namespace mrender