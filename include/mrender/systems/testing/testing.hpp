#pragma once

#include "mrender/systems/system.hpp"

namespace mrender {

class Testing : public RenderSystem
{
public:
    Testing();
    ~Testing();

    bool init(mrender::RenderContext& context) override;
    void render(mrender::RenderContext& context) override;
};

}   // namespace mrender
