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

GfxContext* createGfxContext(const RenderSettings& settings)
{
	return new GfxContextImplementation(settings);
}

}	// namespace mrender