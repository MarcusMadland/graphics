#include "mrender/mrender.hpp"

#include "mrender/handler/shader.hpp"
#include "mrender/handler/render_context.hpp"
#include "mrender/handler/camera.hpp"
#include "mrender/handler/geometry.hpp"
#include "mrender/handler/framebuffer.hpp"

#include <bx/math.h>

namespace mrender {

RenderSystem::RenderSystem(const std::string_view& name)
	: mName(name)
{}

Camera::Camera(const CameraSettings& settings)
	: mSettings(settings)
{}

Geometry::Geometry(const BufferLayout& layout, void* vertexData, uint32_t vertexSize, std::vector<uint16_t> indices)
	: mIndexData(indices), mVertexData(static_cast<uint8_t*>(vertexData))
{}

Renderable::Renderable(std::shared_ptr<Geometry> geometry, const std::string_view& shader)
	: mGeometry(std::move(geometry)), mShader(shader)
{
	bx::mtxIdentity(mTransform);
}

std::shared_ptr<Shader> RenderContext::createShader()
{
	return std::make_shared<ShaderImplementation>();
}

std::shared_ptr<Camera> RenderContext::createCamera(const CameraSettings& settings)
{
	return std::make_shared<CameraImplementation>(settings);
}

std::shared_ptr<Geometry> RenderContext::createGeometry(const BufferLayout& layout, void* vertexData, uint32_t vertexSize, std::vector<uint16_t> indices)
{
	return std::make_shared<GeometryImplementation>(layout, vertexData, vertexSize, indices);
}

std::shared_ptr<Renderable> RenderContext::createRenderable(std::shared_ptr<Geometry> geometry, const std::string_view& shader)
{
	return std::make_shared<Renderable>(std::move(geometry), shader);
}

std::shared_ptr<FrameBuffer> RenderContext::createFrameBuffer(const TextureFormat& format, bool createDepth, uint32_t width, uint32_t height)
{
	return std::make_shared<FrameBufferImplementation>(*this,  format, createDepth, width, height);
}

std::shared_ptr<RenderContext> createRenderContext()
{
	return std::make_shared<RenderContextImplementation>();
}

}	// namespace mrender