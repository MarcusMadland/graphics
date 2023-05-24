#pragma once

#include "mrender/renderers/renderer.hpp"
#include "mrender/techniques/technique.hpp"

namespace Capsaicin
{
    /** The GI1.0 renderer. */
    class GI10Renderer : public Renderer::Registrar<GI10Renderer>
    {
    public:
        static constexpr std::string_view Name = "GI-1.0";

        GI10Renderer() noexcept {}

        std::vector<std::unique_ptr<RenderTechnique>> setupRenderTechniques(
            mrender::RenderContext& context) noexcept override;

    private:
    };
} // namespace Capsaicin

