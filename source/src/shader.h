/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#ifndef GRAPHICS_SHADER_H
#define GRAPHICS_SHADER_H

#include <base/readerwriter.h>

namespace graphics
{
	struct DescriptorType
	{
		enum Enum
		{
			StorageBuffer,
			StorageImage,

			Count
		};
	};

	DescriptorType::Enum idToDescriptorType(uint16_t _id);
	uint16_t descriptorTypeToId(DescriptorType::Enum _type);

	struct TextureComponentType
	{
		enum Enum
		{
			Float,
			Int,
			Uint,
			Depth,
			UnfilterableFloat,

			Count
		};
	};

	TextureComponentType::Enum idToTextureComponentType(uint8_t _id);
	uint8_t textureComponentTypeToId(TextureComponentType::Enum _type);

	struct TextureDimension
	{
		enum Enum
		{
			Dimension1D,
			Dimension2D,
			Dimension2DArray,
			DimensionCube,
			DimensionCubeArray,
			Dimension3D,

			Count
		};
	};

	TextureDimension::Enum idToTextureDimension(uint8_t _id);
	uint8_t textureDimensionToId(TextureDimension::Enum _dim);

	///
	void disassemble(base::WriterI* _writer, base::ReaderSeekerI* _reader, base::Error* _err = NULL);

	///
	void disassemble(base::WriterI* _writer, const void* _data, uint32_t _size, base::Error* _err = NULL);

} // namespace graphics

#endif // GRAPHICS_SHADER_H
