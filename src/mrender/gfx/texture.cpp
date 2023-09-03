#include "mrender/gfx/texture.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

TextureImplementation::TextureImplementation(TextureFormat format, uint64_t textureFlags, uint16_t width, uint16_t height)
	: mFormat(format)
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

TextureImplementation::TextureImplementation(const uint8_t* data, TextureFormat format, uint64_t textureFlags, uint16_t width, uint16_t height, uint16_t channels)
	: mFormat(format)
{
	if (data)
	{
		mHandle = bgfx::createTexture2D(
			width
			, height
			, false
			, 1
			, toBgfx(format)
			, textureFlags
			, bgfx::copy(data, width * height * channels));

		//delete[] data; @todo
	}
	else
	{
		mHandle = BGFX_INVALID_HANDLE;
	}
}

TextureImplementation::~TextureImplementation()
{
	if (bgfx::isValid(mHandle))
	{
		bgfx::destroy(mHandle);
	}
}

bgfx::TextureFormat::Enum TextureImplementation::toBgfx(const TextureFormat& format)
{
	return static_cast<bgfx::TextureFormat::Enum>(static_cast<std::underlying_type_t<TextureFormat>>(format));
}

}	// namespace mrender
