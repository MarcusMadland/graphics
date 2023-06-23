#pragma once

#include "mrender/mrender.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

class TextureImplementation : public Texture
{
	friend class RenderContextImplementation;
	friend class FramebufferImplementation;
	friend class PostProcessing;
	friend class ShadowMapping;
	friend class GBuffer;

public:
	TextureImplementation(TextureFormat format, uint64_t textureFlags, uint16_t width = 0, uint16_t height = 0);
	~TextureImplementation();

	virtual [[nodiscard]] TextureFormat getFormat() const override { return mFormat; }

private:
	bgfx::TextureFormat::Enum toBgfx(const TextureFormat& format);

private:
	bgfx::TextureHandle mHandle;
	TextureFormat mFormat;
};

}	// namespace mrender
