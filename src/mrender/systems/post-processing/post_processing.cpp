#include "mrender/systems/post-processing/post_processing.hpp"

#include <bgfx/bgfx.h> // @todo Make a wrapper around bgfx tags for tags we want to support
#include "mrender/handler/shader.hpp" // @todo Add support for setting shader uniforms
#include "mrender/handler/texture.hpp" // ...

namespace mrender {

PostProcessing::PostProcessing()
    : RenderSystem("Post Processing")
{
}

PostProcessing::~PostProcessing()
{
}

bool PostProcessing::init(RenderContext& context)
{
    // Shader
    context.loadShader("screen", "C:/Users/marcu/Dev/mengine/mrender/shaders/screen");

    // Render state
    mState = context.createRenderState(0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A);

    // Screen quad
    BufferLayout layout =
    { {
        { AttribType::Float, 3, Attrib::Position },
        { AttribType::Float, 2, Attrib::TexCoord0 },
    } };
    mScreenQuad = context.createGeometry(layout, mQuadVertices.data(), static_cast<uint32_t>(mQuadVertices.size() * sizeof(VertexData)), mQuadIndices);

    return true;
}

void PostProcessing::render(RenderContext& context)
{
    // Set current renderpass id
    context.setRenderState(mState);

    // Clear
    context.clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);

    // Set shader uniforms
    context.setParameter("screen", "s_Shadow", context.getBuffers().at("ShadowMap"));
    context.setParameter("screen", "s_Color", context.getBuffers().at("GBufferColor"));

    // Submit quad
    context.submit(mScreenQuad, "screen", nullptr);
}

std::unordered_map<std::string, std::shared_ptr<Texture>> PostProcessing::getBuffers(RenderContext& context)
{
    std::unordered_map<std::string, std::shared_ptr<Texture>> buffers;
    return buffers;
}

}   // namespace mrender