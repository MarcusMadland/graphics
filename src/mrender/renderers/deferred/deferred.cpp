#include "mrender/renderers/deferred/deferred.hpp"
#include "mrender/systems/shadow-mapping/shadow_mapping.hpp"
#include "mrender/systems/deferred/deferred.hpp"
#include "mrender/systems/post-processing/post_processing.hpp"
#include "mrender/systems/empty/empty.hpp"

namespace mrender {

std::vector<std::shared_ptr<RenderSystem>> DeferredRenderer::setupRenderSystems(GfxContext* context)
{
    std::vector<std::shared_ptr<RenderSystem>> render_techniques;
    render_techniques.emplace_back(std::make_shared<ShadowMapping>());
    render_techniques.emplace_back(std::make_shared<Deferred>());
    render_techniques.emplace_back(std::make_shared<PostProcessing>());
   
    // more ..
    return render_techniques;
}

}