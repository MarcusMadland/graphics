#include "mrender/mrender.hpp"
#include "mrender/gfx/render_context.hpp"

namespace mrender {

RenderSystem::RenderSystem(const std::string_view& name)
	: mName(name)
{}

GfxContext* createGfxContext(const RenderSettings& settings)
{
	return new GfxContextImplementation(settings);
}

}	// namespace mrender