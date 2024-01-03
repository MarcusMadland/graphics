/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bimg/blob/master/LICENSE
 */

#ifndef BIMG_CONFIG_H_HEADER_GUARD
#define BIMG_CONFIG_H_HEADER_GUARD

#include <base/base.h>

#ifndef BIMG_DECODE_ENABLE
#	define BIMG_DECODE_ENABLE 1
#endif // BIMG_DECODE_ENABLE

#ifndef BIMG_DECODE_BC1
#	define BIMG_DECODE_BC1 BIMG_DECODE_ENABLE
#endif // BIMG_DECODE_BC1

#ifndef BIMG_DECODE_BC2
#	define BIMG_DECODE_BC2 BIMG_DECODE_ENABLE
#endif // BIMG_DECODE_BC2

#ifndef BIMG_DECODE_BC3
#	define BIMG_DECODE_BC3 BIMG_DECODE_ENABLE
#endif // BIMG_DECODE_BC3

#ifndef BIMG_DECODE_BC4
#	define BIMG_DECODE_BC4 BIMG_DECODE_ENABLE
#endif // BIMG_DECODE_BC4

#ifndef BIMG_DECODE_BC5
#	define BIMG_DECODE_BC5 BIMG_DECODE_ENABLE
#endif // BIMG_DECODE_BC5

#ifndef BIMG_DECODE_BC6
#	define BIMG_DECODE_BC6 BIMG_DECODE_ENABLE
#endif // BIMG_DECODE_BC6

#ifndef BIMG_DECODE_BC7
#	define BIMG_DECODE_BC7 BIMG_DECODE_ENABLE
#endif // BIMG_DECODE_BC7

#ifndef BIMG_DECODE_ATC
#	define BIMG_DECODE_ATC BIMG_DECODE_ENABLE
#endif // BIMG_DECODE_ATC

#ifndef BIMG_DECODE_ASTC
#	define BIMG_DECODE_ASTC BIMG_DECODE_ENABLE
#endif // BIMG_DECODE_ASTC

#ifndef BIMG_DECODE_ETC1
#	define BIMG_DECODE_ETC1 BIMG_DECODE_ENABLE
#endif // BIMG_DECODE_ETC1

#ifndef BIMG_DECODE_ETC2
#	define BIMG_DECODE_ETC2 BIMG_DECODE_ENABLE
#endif // BIMG_DECODE_ETC2

#endif // BIMG_CONFIG_H_HEADER_GUARD