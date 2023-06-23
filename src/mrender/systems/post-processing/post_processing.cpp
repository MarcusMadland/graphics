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
    auto shader = std::static_pointer_cast<ShaderImplementation>(context.getShaders().at("screen"));
    auto shadowMap = std::static_pointer_cast<TextureImplementation>(context.getBuffers().at("ShadowMap"));
    auto gBufferColor = std::static_pointer_cast<TextureImplementation>(context.getBuffers().at("GBufferColor"));
    if (shader->mUniformHandles.count("s_Shadow") > 0 && bgfx::isValid(shader->mUniformHandles.at("s_Shadow").mHandle) && bgfx::isValid(shadowMap->mHandle))
    {
        bgfx::setTexture(shader->mUniformHandles.at("s_Shadow").unit, shader->mUniformHandles.at("s_Shadow").mHandle, shadowMap->mHandle);
    }
    if (shader->mUniformHandles.count("s_Color") > 0 && bgfx::isValid(shader->mUniformHandles.at("s_Color").mHandle) && bgfx::isValid(gBufferColor->mHandle))
    {
        bgfx::setTexture(shader->mUniformHandles.at("s_Color").unit, shader->mUniformHandles.at("s_Color").mHandle, gBufferColor->mHandle);
    }

    // Submit quad
    context.submit(mScreenQuad, "screen", nullptr);
}

std::unordered_map<std::string, std::shared_ptr<Texture>> PostProcessing::getBuffers(RenderContext& context)
{
    std::unordered_map<std::string, std::shared_ptr<Texture>> buffers;
    return buffers;
}

}   // namespace mrender