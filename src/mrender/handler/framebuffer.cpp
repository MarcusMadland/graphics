#include "mrender/handler/framebuffer.hpp"
#include "mrender/handler/texture.hpp"
#include "mrender/handler/render_context.hpp"

namespace mrender {

FrameBufferImplementation::FrameBufferImplementation(RenderContext& context, TextureFormat format, bool createDepth, uint16_t width, uint16_t height)
	: FrameBuffer(context, format, createDepth, width, height)
    , context(context), format(format), createDepth(createDepth), width(width), height(height)
    , mHandle(BGFX_INVALID_HANDLE), mId(0)
{
    createFrameBuffer();
}

FrameBufferImplementation::~FrameBufferImplementation()
{
    destroyFrameBuffer();
}

void FrameBufferImplementation::reset()
{
    //destroyFrameBuffer();
    //createFrameBuffer();
}

void FrameBufferImplementation::reset(uint16_t width, uint16_t height)
{
    //this->width = width; this->height = height;
    //reset();
}

void FrameBufferImplementation::createFrameBuffer()
{
    // Render pass ID
    mId = context.getPassCount();
    context.setPassCount(mId + 1);

    // Create color buffer
    mColorBuffer = std::make_shared<TextureImplementation>(format, BGFX_TEXTURE_RT, width, height);
    bgfx::TextureHandle colorHandle = std::static_pointer_cast<TextureImplementation>(mColorBuffer)->mHandle;

    if (createDepth)
    {
        // Create depth buffer
        mDepthBuffer = std::make_shared<TextureImplementation>(TextureFormat::D16, BGFX_TEXTURE_RT, width, height);
        bgfx::TextureHandle depthHandle = std::static_pointer_cast<TextureImplementation>(mDepthBuffer)->mHandle;

        bgfx::Attachment attachments[2] = { bgfx::Attachment(), bgfx::Attachment() };
        attachments[0].init(colorHandle);
        attachments[1].init(depthHandle, bgfx::Access::Write);
        mHandle = bgfx::createFrameBuffer(2, attachments, false);
    }
    else
    {
        bgfx::Attachment attachments[1] = { bgfx::Attachment() };
        attachments[0].init(colorHandle);
        mHandle = bgfx::createFrameBuffer(1, attachments, false);
    }
}

void FrameBufferImplementation::destroyFrameBuffer()
{
    if (bgfx::isValid(mHandle))
    {
        bgfx::destroy(mHandle);
    }
    if (mColorBuffer)
    {
        mColorBuffer.reset();
    }
    if (mDepthBuffer)
    {
        mDepthBuffer.reset();
    }
}

}