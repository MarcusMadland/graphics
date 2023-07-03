#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class GBuffer : public RenderSystem
{
public:
    GBuffer();
    ~GBuffer();

    bool init(GfxContext* context) override;
    void render(GfxContext* context) override;

    BufferList getBuffers(GfxContext* context) override;

private:
    RenderStateHandle mState;
    BufferList mBuffers;
    FramebufferHandle mFramebuffer;
};

}   // namespace mrender
