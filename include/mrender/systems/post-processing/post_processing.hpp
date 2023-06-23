#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class PostProcessing : public RenderSystem
{
public:
    PostProcessing();
    ~PostProcessing();

    bool init(RenderContext& context) override;
    void render(RenderContext& context) override;

    std::unordered_map<std::string, std::shared_ptr<Texture>> getBuffers(RenderContext& context) override;

private:
    std::shared_ptr<RenderState> mState;
    std::shared_ptr<Geometry> mScreenQuad;

    struct VertexData
    {
        float x;
        float y;
        float z;
        float texX;
        float texY;
    };
    std::vector<VertexData> mQuadVertices =
    {
        { -1.0f,  1.0f, 0.0f,  0.0f,0.0f },
        {  1.0f,  1.0f, 0.0f,  1.0f,0.0f },
        { -1.0f, -1.0f, 0.0f,  0.0f,1.0f },
        {  1.0f, -1.0f, 0.0f,  1.0f,1.0f },
    };
    const std::vector<uint16_t> mQuadIndices =
    {
        0, 1, 2, 1, 3, 2,
    };
};

}   // namespace mrender
