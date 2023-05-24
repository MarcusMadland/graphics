#pragma once

#include "mrender/core/factory.hpp"
#include "mrender/handler/render_context.hpp"
#include "mrender/systems/system.hpp"

#include <memory>
#include <vector>

namespace mrender {
    
class Renderer : public Factory<Renderer>
{
public:
    Renderer(Key) {}

    virtual std::vector<std::unique_ptr<RenderSystem>> setupRenderSystems(mrender::RenderContext& context) = 0;
};

}	// namespace mrender