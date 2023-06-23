#pragma once

#include "mrender/mrender.hpp"

#include "mrender/systems/shadow-mapping/shadow_mapping.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

class GBuffer : public RenderSystem
{
public:
    GBuffer();
    ~GBuffer();

    bool init(RenderContext& context) override;
    void render(RenderContext& context) override;

    std::unordered_map<std::string, std::shared_ptr<Texture>> getBuffers(RenderContext& context) override;

private:
    std::shared_ptr<Framebuffer> mFramebuffer;
    std::shared_ptr<RenderState> mState;
};

}   // namespace mrender
