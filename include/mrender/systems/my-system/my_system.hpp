#pragma once

#include "mrender/mrender.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

struct PosColorVertex
{
    float x;
    float y;
    float z;
    uint32_t abgr;
};

class MySystem : public RenderSystem
{
public:
    MySystem();
    ~MySystem();

    bool init(mrender::RenderContext& context) override;
    void render(mrender::RenderContext& context) override;

private:
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;

    float cam_pitch = 0.0f;
    float cam_yaw = 0.0f;
    float rot_scale = 0.01f;
};

}   // namespace mrender
