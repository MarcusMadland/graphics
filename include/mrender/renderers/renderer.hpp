#pragma once

#include "mrender/core/factory.hpp"
#include "mrender/handler/render_context.hpp"
#include "mrender/techniques/technique.hpp"

#include <memory>
#include <vector>

namespace Capsaicin {
    
class Renderer : public Factory<Renderer>
{
    Renderer(Renderer const&) = delete;
    Renderer& operator=(Renderer const&) = delete;

public:
    Renderer(Key) noexcept {}

    virtual std::vector<std::unique_ptr<RenderTechnique>> setupRenderTechniques(mrender::RenderContext& context) noexcept = 0;
private:
};


}	// namespace mrender