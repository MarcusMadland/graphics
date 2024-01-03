/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg/blob/master/LICENSE
 */

#include "bimg_p.h"

namespace bimg
{
	bool imageParseGnf(ImageContainer& _imageContainer, base::ReaderSeekerI* _reader, base::Error* _err)
	{
		BASE_UNUSED(_imageContainer, _reader, _err);
		BASE_ERROR_SET(_err, BIMG_ERROR, "GNF: not supported.");
		return false;
	}

	ImageContainer* imageParseGnf(base::AllocatorI* _allocator, const void* _src, uint32_t _size, base::Error* _err)
	{
		BASE_UNUSED(_allocator);

		base::MemoryReader reader(_src, _size);

		uint32_t magic;
		base::read(&reader, magic, base::ErrorIgnore{});

		ImageContainer imageContainer;
		if (BIMG_CHUNK_MAGIC_GNF != magic
		|| !imageParseGnf(imageContainer, &reader, _err) )
		{
			return NULL;
		}

		BASE_ERROR_SET(_err, BIMG_ERROR, "GNF: not supported.");
		return NULL;
	}

} // namespace bimg
