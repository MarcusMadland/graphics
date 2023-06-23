#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class ShadowMapping : public RenderSystem
{
public:
    ShadowMapping();
    ~ShadowMapping();

    bool init(RenderContext& context) override;
    void render(RenderContext& context) override;

    std::unordered_map<std::string, std::shared_ptr<Texture>> getBuffers(RenderContext& context) override;

private:
    std::shared_ptr<RenderState> mState;
    std::shared_ptr<Framebuffer> mFramebuffer;
    std::shared_ptr<Camera> mCamera;
};

}   // namespace mrender
