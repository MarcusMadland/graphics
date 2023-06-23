#include "mrender/renderers/my-renderer/my_renderer.hpp"
#include "mrender/systems/shadow-mapping/shadow_mapping.hpp"
#include "mrender/systems/gbuffer/gbuffer.hpp"
#include "mrender/systems/post-processing/post_processing.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

namespace mrender {

std::vector<std::shared_ptr<RenderSystem>> MyRenderer::setupRenderSystems(RenderContext& context)
{
    std::vector<std::shared_ptr<RenderSystem>> render_techniques;
    render_techniques.emplace_back(std::make_shared<ShadowMapping>());
    render_techniques.emplace_back(std::make_shared<GBuffer>());
    render_techniques.emplace_back(std::make_shared<PostProcessing>());
    // more ..
    return render_techniques;
}

}