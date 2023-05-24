#pragma once

#include "mrender/handler/render_context.hpp"

namespace mrender {

class RenderSystem
{
public:
    virtual bool init(mrender::RenderContext& mrender) = 0;
    virtual void render(mrender::RenderContext& mrender) = 0;
};

}   // namespace mrender