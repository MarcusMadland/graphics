#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class MySystem2 : public RenderSystem
{
public:
    MySystem2();
    ~MySystem2();

    bool init(mrender::RenderContext& context) override;
    void render(mrender::RenderContext& context) override;
};

}   // namespace mrender
