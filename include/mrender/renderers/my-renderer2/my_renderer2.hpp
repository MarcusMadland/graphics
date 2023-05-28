#pragma once

#include "mrender/renderers/renderer.hpp"
#include "mrender/systems/system.hpp"

namespace mrender {

class MyRenderer2 : public Renderer::Registrar<MyRenderer2>
{
public:
    static constexpr std::string_view Name = "MyRenderer2";

    MyRenderer2() {}

    std::vector<std::unique_ptr<RenderSystem>> setupRenderSystems(
        mrender::RenderContext& context) override;

private:
};

}   // namespace mrender

