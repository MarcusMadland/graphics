#include "mrender/mrender.hpp"

#include "mrender/handler/shader.hpp"
#include "mrender/handler/render_context.hpp"

namespace mrender {

	RenderSystem::RenderSystem(const std::string_view& name)
		: mName(name)
	{}

	std::unique_ptr<RenderContext> createRenderContext()
	{
		return std::make_unique<RenderContextImplementation>();
	}

	std::unique_ptr<Shader> createShader()
	{
		return std::make_unique<ShaderImplementation>();
	}

}