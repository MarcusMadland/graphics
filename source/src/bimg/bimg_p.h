/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg/blob/master/LICENSE
 */

#ifndef BIMG_P_H_HEADER_GUARD
#define BIMG_P_H_HEADER_GUARD

#ifndef BASE_CONFIG_DEBUG
#	error "BASE_CONFIG_DEBUG must be defined in build script!"
#endif // BIMG_CONFIG_DEBUG

#if BASE_CONFIG_DEBUG
#	define BASE_TRACE  _BIMG_TRACE
#	define BASE_WARN   _BIMG_WARN
#	define BASE_ASSERT _BIMG_ASSERT
#endif // BASE_CONFIG_DEBUG

#define BASE_ASSERT2 BASE_ASSERT

#define _BIMG_TRACE(_format, ...)                                                                  \
	BASE_MACRO_BLOCK_BEGIN                                                                           \
		base::debugPrintf(__FILE__ "(" BASE_STRINGIZE(__LINE__) "): BX " _format "\n", ##__VA_ARGS__); \
	BASE_MACRO_BLOCK_END

#define _BIMG_WARN(_condition, _format, ...)          \
	BASE_MACRO_BLOCK_BEGIN                              \
		if (!BASE_IGNORE_C4127(_condition) )            \
		{                                             \
			BASE_TRACE("WARN " _format, ##__VA_ARGS__); \
		}                                             \
	BASE_MACRO_BLOCK_END

#define _BIMG_ASSERT(_condition, _format, ...)          \
	BASE_MACRO_BLOCK_BEGIN                                \
		if (!BASE_IGNORE_C4127(_condition) )              \
		{                                               \
			BASE_TRACE("ASSERT " _format, ##__VA_ARGS__); \
			base::debugBreak();                           \
		}                                               \
	BASE_MACRO_BLOCK_END

#include <bimg/bimg.h>
#include <base/allocator.h>
#include <base/debug.h>
#include <base/readerwriter.h>
#include <base/pixelformat.h>
#include <base/endian.h>
#include <base/error.h>
#include <base/simd_t.h>

#include "config.h"

#define BIMG_CHUNK_MAGIC_TEX BASE_MAKEFOURCC('T', 'E', 'X', 0x0)
#define BIMG_CHUNK_MAGIC_GNF BASE_MAKEFOURCC('G', 'N', 'F', ' ')

BASE_ERROR_RESULT(BIMG_ERROR, BASE_MAKEFOURCC('b', 'i', 'm', 'g') );

#ifndef BIMG_CONFIG_ASTC_DECODE
#	define BIMG_CONFIG_ASTC_DECODE 0
#endif // BIMG_CONFIG_ASTC_DECODE

namespace bimg
{
	struct Memory
	{
		uint8_t* data;
		uint32_t size;
	};

	struct TextureCreate
	{
		TextureFormat::Enum m_format;
		uint16_t m_width;
		uint16_t m_height;
		uint16_t m_depth;
		uint16_t m_numLayers;
		uint8_t m_numMips;
		bool m_cubeMap;
		const Memory* m_mem;
	};

	inline uint8_t calcNumMips(bool _hasMips, uint16_t _width, uint16_t _height, uint16_t _depth = 1)
	{
		if (_hasMips)
		{
			const uint32_t max = base::max(_width, _height, _depth);
			const uint32_t num = 1 + uint32_t(base::log2((int32_t)max) );

			return uint8_t(num);
		}

		return 1;
	}

	///
	void imageConvert(
		  void* _dst
		, uint32_t _bpp
		, base::PackFn _pack
		, const void* _src
		, base::UnpackFn _unpack
		, uint32_t _size
		);

	///
	void imageConvert(
		  void* _dst
		, uint32_t _dstBpp
		, base::PackFn _pack
		, const void* _src
		, uint32_t _srcBpp
		, base::UnpackFn _unpack
		, uint32_t _width
		, uint32_t _height
		, uint32_t _srcPitch
		);

	///
	bool imageParseGnf(
		  ImageContainer& _imageContainer
		, base::ReaderSeekerI* _reader
		, base::Error* _err
		);

} // namespace bimg

#endif // BIMG_P_H_HEADER_GUARD
