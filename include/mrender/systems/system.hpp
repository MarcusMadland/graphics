#pragma once

#include "mrender/handler/render_context.hpp"

#include <string_view>

namespace mrender {

class RenderSystem
{
public:
    RenderSystem(const std::string_view& name);

    [[nodiscard]] std::string_view getName() const { return mName; }

    virtual bool init(const mrender::RenderContext& context) = 0;
    virtual void render(mrender::RenderContext& context) = 0;

private:
    std::string_view mName;
};

}   // namespace mrender