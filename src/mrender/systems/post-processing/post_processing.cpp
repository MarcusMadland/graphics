#include "mrender/systems/post-processing/post_processing.hpp"

#include "mrender/handler/shader.hpp"
#include "mrender/handler/render_context.hpp"
#include "mrender/handler/texture.hpp"

#include <bgfx/bgfx.h>
#include <bx/math.h>

#define RENDER_POSTPROCESS_PASS_ID 2

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

    // Screen quad
    BufferLayout layout =
    { {
        { AttribType::Float, 3, Attrib::Position },
        { AttribType::Float, 2, Attrib::TexCoord0 },
    } };
    mScreenQuad = context.createGeometry(layout, mQuadVertices.data(), static_cast<uint32_t>(mQuadVertices.size() * sizeof(VertexData)), mQuadIndices);

    // Render State
    auto shader = std::static_pointer_cast<ShaderImplementation>(context.getShaders().at("screen"));
    mState[0] = RenderState();
    mState[0].m_state = (0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A);
    mState[0].m_viewId = RENDER_POSTPROCESS_PASS_ID;

    return true;
}

void PostProcessing::render(RenderContext& context)
{
    bgfx::setViewRect(mState[0].m_viewId, 0, 0, bgfx::BackbufferRatio::Equal);
    bgfx::setViewClear(mState[0].m_viewId
        , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
        , 0xff00ffff, 1.0f, 0
    );

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
   
    // Set current renderpass id
    RenderContextImplementation& contextCasted = static_cast<RenderContextImplementation&>(context);
    contextCasted.mCurrentRenderPass = mState[0].m_viewId;

    // Set state
    bgfx::setState(mState[0].m_state);

    // Submit quad
    context.submit(mScreenQuad, "screen", nullptr);
}

std::unordered_map<std::string, std::shared_ptr<Texture>> PostProcessing::getBuffers(RenderContext& context)
{
    std::unordered_map<std::string, std::shared_ptr<Texture>> buffers;
    return buffers;
}

}   // namespace mrender