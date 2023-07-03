#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class Deferred : public Renderer::Registrar<Deferred>
{
public:
    static constexpr std::string_view Name = "Deferred";

    Deferred() {}

    std::vector<std::shared_ptr<RenderSystem>> setupRenderSystems(
        mrender::GfxContext* context) override;
};

}   // namespace mrender

