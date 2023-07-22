#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class ShadowMapping : public RenderSystem
{
public:
    ShadowMapping(GfxContext* context);
    ~ShadowMapping();

    bool init(GfxContext* context) override;
    void render(GfxContext* context) override;

private:
    ShaderHandle mShader;
    RenderStateHandle mState;
    FramebufferHandle mFramebuffer;
    CameraHandle mCamera;
};

}   // namespace mrender
