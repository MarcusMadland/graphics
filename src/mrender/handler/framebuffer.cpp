#include "mrender/handler/framebuffer.hpp"
#include "mrender/handler/texture.hpp"

namespace mrender {

FrameBufferImplementation::FrameBufferImplementation(RenderContext& context, TextureFormat format, bool createDepth, uint16_t width, uint16_t height)
	: FrameBuffer(context, format, createDepth, width, height)
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
        mDepthBuffer = std::make_shared<TextureImplementation>(TextureFormat::D24S8, BGFX_TEXTURE_RT, width, height);
        bgfx::TextureHandle depthHandle = std::static_pointer_cast<TextureImplementation>(mDepthBuffer)->mHandle;

        bgfx::Attachment attachments[2];
        attachments[0].init(colorHandle);
        attachments[1].init(depthHandle, bgfx::Access::Write);
        mHandle = bgfx::createFrameBuffer(2, attachments, true);
    }
    else
    {
        bgfx::Attachment attachments[1];
        attachments[0].init(colorHandle);
        mHandle = bgfx::createFrameBuffer(1, attachments, true);
    }
}

}