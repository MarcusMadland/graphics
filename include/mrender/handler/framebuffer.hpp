#pragma once

#include "mrender/mrender.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

class FrameBufferImplementation : public FrameBuffer
{
	friend class RenderContextImplementation;

public:
	FrameBufferImplementation(RenderContext& context, TextureFormat format, bool createDepth = false, uint16_t width = 0, uint16_t height = 0);

	virtual [[nodiscard]] std::shared_ptr<Texture> getColorBuffer() const override { return mColorBuffer; }
	virtual [[nodiscard]] std::shared_ptr<Texture> getDepthBuffer() const override { return mDepthBuffer; }

private:
	bgfx::FrameBufferHandle mHandle;
	std::shared_ptr<Texture> mColorBuffer;
	std::shared_ptr<Texture> mDepthBuffer;
	uint16_t mId;
};

}