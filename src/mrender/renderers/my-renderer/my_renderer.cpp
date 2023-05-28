#include "mrender/renderers/my-renderer/my_renderer.hpp"
#include "mrender/systems/my-system/my_system.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

namespace mrender {

std::vector<std::unique_ptr<RenderSystem>> MyRenderer::setupRenderSystems(RenderContext& context)
{
    std::vector<std::unique_ptr<RenderSystem>> render_techniques;
    render_techniques.emplace_back(std::make_unique<MySystem>());
    // more ..
    return render_techniques;
}

}