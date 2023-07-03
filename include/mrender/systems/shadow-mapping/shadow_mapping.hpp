#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class ShadowMapping : public RenderSystem
{
public:
    ShadowMapping();
    ~ShadowMapping();

    bool init(GfxContext* context) override;
    void render(GfxContext* context) override;

    BufferList getBuffers(GfxContext* context) override;

private:
    ShaderHandle mShader;
    RenderStateHandle mState;
    BufferList mBuffers;
    FramebufferHandle mFramebuffer;
    CameraHandle mCamera;
};

}   // namespace mrender
