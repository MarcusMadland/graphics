#pragma once

#include "mrender/mrender.hpp"

#include "mrender/systems/shadow-mapping/shadow_mapping.hpp" // @todo temp

namespace mrender {


struct VertexData
{
    float x;
    float y;
    float z;
    float texX;
    float texY;
};

class PostProcessing : public RenderSystem
{
public:
    PostProcessing();
    ~PostProcessing();

    bool init(RenderContext& context) override;
    void render(RenderContext& context) override;

    std::unordered_map<std::string, std::shared_ptr<Texture>> getBuffers(RenderContext& context) override;

private:
    std::shared_ptr<Geometry> mScreenQuad;
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
    std::shared_ptr<RenderState> mState;
};

}   // namespace mrender
