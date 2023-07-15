#include "mrender/systems/post-processing/post_processing.hpp"

#include <bgfx/bgfx.h> // @todo Make a wrapper around bgfx tags for tags we want to support
#include "mrender/gfx/shader.hpp" // @todo Add support for setting shader uniforms
#include "mrender/gfx/texture.hpp" // ...

namespace mrender {

PostProcessing::PostProcessing()
    : RenderSystem("Post Processing"), mShader(INVALID_HANDLE), mState(INVALID_HANDLE), mScreenQuad(INVALID_HANDLE)
{
}

PostProcessing::~PostProcessing()
{
}

bool PostProcessing::init(GfxContext* context)
{
    // Shader
    mShader = context->createShader(
        "C:/Users/marcu/Dev/mengine/mrender/shaders/build/screen-vert.bin",
        "C:/Users/marcu/Dev/mengine/mrender/shaders/build/screen-frag.bin");

    // Render state
    mState = context->createRenderState("Post Processing", 0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A);

    // Screen quad
    BufferLayout layout =
    {
        { BufferElement::AttribType::Float, BufferElement::Attrib::Position, 3 },
        { BufferElement::AttribType::Float, BufferElement::Attrib::TexCoord0, 2 },
    };
    mScreenQuad = context->createGeometry(layout, mQuadVertices.data(), static_cast<uint32_t>(mQuadVertices.size() * sizeof(VertexData)), mQuadIndices);

    return true;
}

void PostProcessing::render(GfxContext* context)
{
    PROFILE_SCOPE(mName);

    // Set current renderpass id
    context->setActiveRenderState(mState);
    context->clear(BGFX_CLEAR_COLOR);

    // Set shader uniforms
    TextureHandle diffuseBuffer = context->getSharedBuffers().at("Light");
    context->setTexture(mShader, "u_color", diffuseBuffer, 1);

    // Submit quad
    context->submit(mScreenQuad, mShader, INVALID_HANDLE);
}

BufferList PostProcessing::getBuffers(GfxContext* context)
{
    BufferList buffers;
    return buffers;
}

UniformDataList PostProcessing::getUniformData(GfxContext* context)
{
    return UniformDataList();
}

}   // namespace mrender