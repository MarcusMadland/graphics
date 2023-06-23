#pragma once

#include "mrender/mrender.hpp"

#include <bgfx/bgfx.h>

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
    std::shared_ptr<Camera> mCamera;
	std::shared_ptr<Framebuffer> mFramebuffer;
    std::shared_ptr<RenderState> mState;
};

}   // namespace mrender
