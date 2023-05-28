#pragma once

#include "mrender/systems/system.hpp"

namespace mrender {

class MySystem2 : public RenderSystem
{
public:
    MySystem2();
    ~MySystem2();

    bool init(const mrender::RenderContext& context) override;
    void render(mrender::RenderContext& context) override;
};

}   // namespace mrender
