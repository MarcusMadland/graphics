#include "mrender/renderers/gi/gi.hpp"
#include "mrender/techniques/testing/testing.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

namespace Capsaicin {

std::vector<std::unique_ptr<RenderTechnique>> Capsaicin::GI10Renderer::setupRenderTechniques(mrender::RenderContext& context) noexcept

{
    std::vector<std::unique_ptr<RenderTechnique>> render_techniques;
    render_techniques.emplace_back(std::make_unique<Testing>());
    // more ..
    return render_techniques;
}

}