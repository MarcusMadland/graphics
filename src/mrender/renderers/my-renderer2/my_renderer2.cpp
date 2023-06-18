#include "mrender/renderers/my-renderer2/my_renderer2.hpp"
#include "mrender/systems/my-system/my_system.hpp"
#include "mrender/systems/my-system2/my_system2.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

namespace mrender {

std::vector<std::shared_ptr<RenderSystem>> MyRenderer2::setupRenderSystems(RenderContext& context)
{
    std::vector<std::shared_ptr<RenderSystem>> render_techniques;
    render_techniques.emplace_back(std::make_shared<MySystem>());
    render_techniques.emplace_back(std::make_shared<MySystem2>());
    // more ..
    return render_techniques;
}

}