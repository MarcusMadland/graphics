#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class MyRenderer2 : public Renderer::Registrar<MyRenderer2>
{
public:
    static constexpr std::string_view Name = "MyRenderer2";

    MyRenderer2() {}

    std::vector<std::shared_ptr<RenderSystem>> setupRenderSystems(
        mrender::GfxContext* context) override;

private:
};

}   // namespace mrender

