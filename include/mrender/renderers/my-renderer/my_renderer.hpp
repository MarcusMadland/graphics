#pragma once

#include "mrender/renderers/renderer.hpp"
#include "mrender/systems/system.hpp"

namespace mrender {

class MyRenderer : public Renderer::Registrar<MyRenderer>
{
public:
    static constexpr std::string_view Name = "MyRenderer";

    MyRenderer() {}

    std::vector<std::unique_ptr<RenderSystem>> setupRenderSystems(
        mrender::RenderContext& context) override;

private:
};

}   // namespace mrender

