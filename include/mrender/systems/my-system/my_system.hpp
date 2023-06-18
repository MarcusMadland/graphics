#pragma once

#include "mrender/mrender.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

struct VertexData
{
    float x;
    float y;
    float z;
    float texX;
    float texY;
};

static std::vector<VertexData> quadVertices =
{
    { -1.0f,  1.0f, 0.0f,  0.0f,0.0f },
    {  1.0f,  1.0f, 0.0f,  1.0f,0.0f },
    { -1.0f, -1.0f, 0.0f,  0.0f,1.0f },
    {  1.0f, -1.0f, 0.0f,  1.0f,1.0f },
};

static const std::vector<uint16_t> quadIndices =
{
    0, 1, 2, 1, 3, 2,
};

class MySystem : public RenderSystem
{
public:
    MySystem();
    ~MySystem();

    bool init(mrender::RenderContext& context) override;
    void render(mrender::RenderContext& context) override;
    std::vector<std::pair<std::string, std::shared_ptr<FrameBuffer>>> getBuffers(RenderContext& context) override;

private:
    std::shared_ptr<Camera> mCamera;
    std::shared_ptr<RenderPass> mPassShadow;
    std::shared_ptr<RenderPass> mPassScene;
    std::shared_ptr<RenderPass> mPassPostProcess;
    std::shared_ptr<Geometry> mScreenQuad;
};

}   // namespace mrender
