#pragma once

#include "mrender/mrender.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

class TextureImplementation : public Texture
{
	friend class GfxContextImplementation;
	friend class FramebufferImplementation;
	friend class PostProcessing;
	friend class ShadowMapping;
	friend class GBuffer;

public:
	TextureImplementation(TextureFormat format, uint64_t textureFlags, uint16_t width = 0, uint16_t height = 0);
	TextureImplementation(const uint8_t* data, TextureFormat format, uint64_t textureFlags, uint16_t width, uint16_t height, uint16_t channels);
	~TextureImplementation();

	virtual [[nodiscard]] TextureFormat getFormat() const { return mFormat; }
	virtual [[nodiscard]] uint16_t getTextureID() const { return mHandle.idx; }

private:
	bgfx::TextureFormat::Enum toBgfx(const TextureFormat& format);

private:
	bgfx::TextureHandle mHandle = BGFX_INVALID_HANDLE;
	TextureFormat mFormat = TextureFormat::Count;
};

}	// namespace mrender
