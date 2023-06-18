#include "mrender/handler/texture.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

TextureImplementation::TextureImplementation(TextureFormat format, uint64_t textureFlags, uint16_t width, uint16_t height)
	: Texture(format, textureFlags, width, height)
	, mFormat(format)
{
	if (width + height != 0)
	{
		mHandle = bgfx::createTexture2D(width, height, false, 1, toBgfx(format), textureFlags);
	}
	else
	{
		mHandle = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, toBgfx(format), textureFlags);
	}
	
}

TextureImplementation::~TextureImplementation()
{
	bgfx::destroy(mHandle);
}

bgfx::TextureFormat::Enum TextureImplementation::toBgfx(const TextureFormat& format)
{
	return static_cast<bgfx::TextureFormat::Enum>(static_cast<std::underlying_type_t<TextureFormat>>(format));
}

}	// namespace mrender
