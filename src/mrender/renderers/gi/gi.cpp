#include "mrender/renderers/gi/gi.hpp"
#include "mrender/systems/testing/testing.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

namespace mrender {

std::vector<std::unique_ptr<RenderSystem>> MyRenderer::setupRenderSystems(RenderContext& context)
{
    std::vector<std::unique_ptr<RenderSystem>> render_techniques;
    render_techniques.emplace_back(std::make_unique<Testing>());
    // more ..
    return render_techniques;
}

}