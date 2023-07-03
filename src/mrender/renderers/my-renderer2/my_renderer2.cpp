#include "mrender/renderers/my-renderer2/my_renderer2.hpp"

#include "mrender/systems/empty/empty.hpp"
#include "mrender/systems/gbuffer/gbuffer.hpp"

namespace mrender {

std::vector<std::shared_ptr<RenderSystem>> MyRenderer2::setupRenderSystems(GfxContext* context)
{
    std::vector<std::shared_ptr<RenderSystem>> render_techniques;
    //render_techniques.emplace_back(std::make_shared<Empty>());
    render_techniques.emplace_back(std::make_shared<GBuffer>());
    return render_techniques;
}

}