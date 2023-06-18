#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class MySystem2 : public RenderSystem
{
public:
    MySystem2();
    ~MySystem2();

    bool init(RenderContext& context) override;
    void render(RenderContext& context) override;
    std::vector<std::pair<std::string, std::shared_ptr<FrameBuffer>>> getBuffers(RenderContext& context) override;

private:
    std::shared_ptr<Camera> mCamera;
    std::shared_ptr<RenderPass> mFirstPass;
};

}   // namespace mrender
