/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#ifndef GRAPHICS_CONFIG_H_HEADER_GUARD
#define GRAPHICS_CONFIG_H_HEADER_GUARD

#include <base/base.h> // base::isPowerOf2

// # Configuration options for graphics.
//
// Any of `GRAPHICS_CONFIG_*` options that's inside `#ifndef` block can be configured externally
// via compiler options.
//
// When selecting rendering backends select all backends you want to include in the build.

#ifndef BASE_CONFIG_DEBUG
#	error "BASE_CONFIG_DEBUG must be defined in build script!"
#endif // BASE_CONFIG_DEBUG

#if !defined(GRAPHICS_CONFIG_RENDERER_AGC)        \
 && !defined(GRAPHICS_CONFIG_RENDERER_DIRECT3D9)  \
 && !defined(GRAPHICS_CONFIG_RENDERER_DIRECT3D11) \
 && !defined(GRAPHICS_CONFIG_RENDERER_DIRECT3D12) \
 && !defined(GRAPHICS_CONFIG_RENDERER_GNM)        \
 && !defined(GRAPHICS_CONFIG_RENDERER_METAL)      \
 && !defined(GRAPHICS_CONFIG_RENDERER_NVN)        \
 && !defined(GRAPHICS_CONFIG_RENDERER_OPENGL)     \
 && !defined(GRAPHICS_CONFIG_RENDERER_OPENGLES)   \
 && !defined(GRAPHICS_CONFIG_RENDERER_VULKAN)     \
 && !defined(GRAPHICS_CONFIG_RENDERER_WEBGPU)

#	ifndef GRAPHICS_CONFIG_RENDERER_AGC
#		define GRAPHICS_CONFIG_RENDERER_AGC (0 \
					|| BASE_PLATFORM_PS5     \
					? 1 : 0)
#	endif // GRAPHICS_CONFIG_RENDERER_AGC

#	ifndef GRAPHICS_CONFIG_RENDERER_DIRECT3D9
#		define GRAPHICS_CONFIG_RENDERER_DIRECT3D9 (0 \
					|| BASE_PLATFORM_LINUX         \
					|| BASE_PLATFORM_WINDOWS       \
					? 1 : 0)
#	endif // GRAPHICS_CONFIG_RENDERER_DIRECT3D9

#	ifndef GRAPHICS_CONFIG_RENDERER_DIRECT3D11
#		define GRAPHICS_CONFIG_RENDERER_DIRECT3D11 (0 \
					|| BASE_PLATFORM_LINUX          \
					|| BASE_PLATFORM_WINDOWS        \
					|| BASE_PLATFORM_WINRT          \
					|| BASE_PLATFORM_XBOXONE        \
					? 1 : 0)
#	endif // GRAPHICS_CONFIG_RENDERER_DIRECT3D11

#	ifndef GRAPHICS_CONFIG_RENDERER_DIRECT3D12
#		define GRAPHICS_CONFIG_RENDERER_DIRECT3D12 (0 \
					|| BASE_PLATFORM_LINUX          \
					|| BASE_PLATFORM_WINDOWS        \
					|| BASE_PLATFORM_WINRT          \
					|| BASE_PLATFORM_XBOXONE        \
					? 1 : 0)
#	endif // GRAPHICS_CONFIG_RENDERER_DIRECT3D12

#	ifndef GRAPHICS_CONFIG_RENDERER_GNM
#		define GRAPHICS_CONFIG_RENDERER_GNM (0 \
					|| BASE_PLATFORM_PS4     \
					? 1 : 0)
#	endif // GRAPHICS_CONFIG_RENDERER_GNM

#	ifndef GRAPHICS_CONFIG_RENDERER_METAL
#		define GRAPHICS_CONFIG_RENDERER_METAL (0           \
					|| (BASE_PLATFORM_IOS && BASE_CPU_ARM) \
					|| (BASE_PLATFORM_IOS && BASE_CPU_X86) \
					|| (BASE_PLATFORM_OSX >= 101100)     \
					? 1 : 0)
#	endif // GRAPHICS_CONFIG_RENDERER_METAL

#	ifndef GRAPHICS_CONFIG_RENDERER_NVN
#		define GRAPHICS_CONFIG_RENDERER_NVN (0 \
					|| BASE_PLATFORM_NX      \
					? 1 : 0)
#	endif // GRAPHICS_CONFIG_RENDERER_NVN

#	ifndef GRAPHICS_CONFIG_RENDERER_OPENGL_MIN_VERSION
#		define GRAPHICS_CONFIG_RENDERER_OPENGL_MIN_VERSION 1
#	endif // GRAPHICS_CONFIG_RENDERER_OPENGL_MIN_VERSION

#	ifndef GRAPHICS_CONFIG_RENDERER_OPENGL
#		define GRAPHICS_CONFIG_RENDERER_OPENGL (0 \
					|| BASE_PLATFORM_BSD        \
					|| BASE_PLATFORM_LINUX      \
					|| BASE_PLATFORM_WINDOWS    \
					? GRAPHICS_CONFIG_RENDERER_OPENGL_MIN_VERSION : 0)
#	endif // GRAPHICS_CONFIG_RENDERER_OPENGL

#	ifndef GRAPHICS_CONFIG_RENDERER_OPENGLES_MIN_VERSION
#		define GRAPHICS_CONFIG_RENDERER_OPENGLES_MIN_VERSION (0 \
					|| BASE_PLATFORM_ANDROID                  \
					? 30 : 1)
#	endif // GRAPHICS_CONFIG_RENDERER_OPENGLES_MIN_VERSION

#	ifndef GRAPHICS_CONFIG_RENDERER_OPENGLES
#		define GRAPHICS_CONFIG_RENDERER_OPENGLES (0 \
					|| BASE_PLATFORM_ANDROID      \
					|| BASE_PLATFORM_EMSCRIPTEN   \
					|| BASE_PLATFORM_RPI          \
					|| BASE_PLATFORM_NX           \
					? GRAPHICS_CONFIG_RENDERER_OPENGLES_MIN_VERSION : 0)
#	endif // GRAPHICS_CONFIG_RENDERER_OPENGLES

#	ifndef GRAPHICS_CONFIG_RENDERER_VULKAN
#		define GRAPHICS_CONFIG_RENDERER_VULKAN (0 \
					|| BASE_PLATFORM_ANDROID    \
					|| BASE_PLATFORM_LINUX      \
					|| BASE_PLATFORM_WINDOWS    \
					|| BASE_PLATFORM_NX         \
					|| BASE_PLATFORM_OSX        \
					? 1 : 0)
#	endif // GRAPHICS_CONFIG_RENDERER_VULKAN

#	ifndef GRAPHICS_CONFIG_RENDERER_WEBGPU
#		define GRAPHICS_CONFIG_RENDERER_WEBGPU 0
#	endif // GRAPHICS_CONFIG_RENDERER_WEBGPU

#else
#	ifndef GRAPHICS_CONFIG_RENDERER_AGC
#		define GRAPHICS_CONFIG_RENDERER_AGC 0
#	endif // GRAPHICS_CONFIG_RENDERER_AGC

#	ifndef GRAPHICS_CONFIG_RENDERER_DIRECT3D9
#		define GRAPHICS_CONFIG_RENDERER_DIRECT3D9 0
#	endif // GRAPHICS_CONFIG_RENDERER_DIRECT3D9

#	ifndef GRAPHICS_CONFIG_RENDERER_DIRECT3D11
#		define GRAPHICS_CONFIG_RENDERER_DIRECT3D11 0
#	endif // GRAPHICS_CONFIG_RENDERER_DIRECT3D11

#	ifndef GRAPHICS_CONFIG_RENDERER_DIRECT3D12
#		define GRAPHICS_CONFIG_RENDERER_DIRECT3D12 0
#	endif // GRAPHICS_CONFIG_RENDERER_DIRECT3D12

#	ifndef GRAPHICS_CONFIG_RENDERER_GNM
#		define GRAPHICS_CONFIG_RENDERER_GNM 0
#	endif // GRAPHICS_CONFIG_RENDERER_GNM

#	ifndef GRAPHICS_CONFIG_RENDERER_METAL
#		define GRAPHICS_CONFIG_RENDERER_METAL 0
#	endif // GRAPHICS_CONFIG_RENDERER_METAL

#	ifndef GRAPHICS_CONFIG_RENDERER_NVN
#		define GRAPHICS_CONFIG_RENDERER_NVN 0
#	endif // GRAPHICS_CONFIG_RENDERER_NVN

#	ifndef GRAPHICS_CONFIG_RENDERER_OPENGL
#		define GRAPHICS_CONFIG_RENDERER_OPENGL 0
#	endif // GRAPHICS_CONFIG_RENDERER_OPENGL

#	ifndef GRAPHICS_CONFIG_RENDERER_OPENGLES
#		define GRAPHICS_CONFIG_RENDERER_OPENGLES 0
#	endif // GRAPHICS_CONFIG_RENDERER_OPENGLES

#	ifndef GRAPHICS_CONFIG_RENDERER_VULKAN
#		define GRAPHICS_CONFIG_RENDERER_VULKAN 0
#	endif // GRAPHICS_CONFIG_RENDERER_VULKAN

#	ifndef GRAPHICS_CONFIG_RENDERER_WEBGPU
#		define GRAPHICS_CONFIG_RENDERER_WEBGPU 0
#	endif // GRAPHICS_CONFIG_RENDERER_VULKAN
#endif // !defined...

#if GRAPHICS_CONFIG_RENDERER_OPENGL && GRAPHICS_CONFIG_RENDERER_OPENGL < 21
#	undef GRAPHICS_CONFIG_RENDERER_OPENGL
#	define GRAPHICS_CONFIG_RENDERER_OPENGL 21
#endif // GRAPHICS_CONFIG_RENDERER_OPENGL && GRAPHICS_CONFIG_RENDERER_OPENGL < 21

#if GRAPHICS_CONFIG_RENDERER_OPENGLES && GRAPHICS_CONFIG_RENDERER_OPENGLES < 20
#	undef GRAPHICS_CONFIG_RENDERER_OPENGLES
#	define GRAPHICS_CONFIG_RENDERER_OPENGLES 20
#endif // GRAPHICS_CONFIG_RENDERER_OPENGLES && GRAPHICS_CONFIG_RENDERER_OPENGLES < 20

#if GRAPHICS_CONFIG_RENDERER_OPENGL && GRAPHICS_CONFIG_RENDERER_OPENGLES
#	error "Can't define both GRAPHICS_CONFIG_RENDERER_OPENGL and GRAPHICS_CONFIG_RENDERER_OPENGLES"
#endif // GRAPHICS_CONFIG_RENDERER_OPENGL && GRAPHICS_CONFIG_RENDERER_OPENGLES

/// Enable use of extensions.
#ifndef GRAPHICS_CONFIG_RENDERER_USE_EXTENSIONS
#	define GRAPHICS_CONFIG_RENDERER_USE_EXTENSIONS 1
#endif // GRAPHICS_CONFIG_RENDERER_USE_EXTENSIONS

/// Enable use of tinystl.
#ifndef GRAPHICS_CONFIG_USE_TINYSTL
#	define GRAPHICS_CONFIG_USE_TINYSTL 0
#endif // GRAPHICS_CONFIG_USE_TINYSTL

/// Debug text maximum scale factor.
#ifndef GRAPHICS_CONFIG_DEBUG_TEXT_MAX_SCALE
#	define GRAPHICS_CONFIG_DEBUG_TEXT_MAX_SCALE 4
#endif // GRAPHICS_CONFIG_DEBUG_TEXT_MAX_SCALE

/// Enable nVidia PerfHUD integration.
#ifndef GRAPHICS_CONFIG_DEBUG_PERFHUD
#	define GRAPHICS_CONFIG_DEBUG_PERFHUD 0
#endif // GRAPHICS_CONFIG_DEBUG_NVPERFHUD

/// Enable annotation for graphics debuggers.
#ifndef GRAPHICS_CONFIG_DEBUG_ANNOTATION
#	define GRAPHICS_CONFIG_DEBUG_ANNOTATION GRAPHICS_CONFIG_DEBUG
#endif // GRAPHICS_CONFIG_DEBUG_ANNOTATION

/// Enable DX11 object names.
#ifndef GRAPHICS_CONFIG_DEBUG_OBJECT_NAME
#	define GRAPHICS_CONFIG_DEBUG_OBJECT_NAME GRAPHICS_CONFIG_DEBUG_ANNOTATION
#endif // GRAPHICS_CONFIG_DEBUG_OBJECT_NAME

/// Enable uniform debug checks.
#ifndef GRAPHICS_CONFIG_DEBUG_UNIFORM
#	define GRAPHICS_CONFIG_DEBUG_UNIFORM GRAPHICS_CONFIG_DEBUG
#endif // GRAPHICS_CONFIG_DEBUG_UNIFORM

/// Enable occlusion debug checks.
#ifndef GRAPHICS_CONFIG_DEBUG_OCCLUSION
#	define GRAPHICS_CONFIG_DEBUG_OCCLUSION GRAPHICS_CONFIG_DEBUG
#endif // GRAPHICS_CONFIG_DEBUG_OCCLUSION

#ifndef GRAPHICS_CONFIG_MULTITHREADED
#	define GRAPHICS_CONFIG_MULTITHREADED ( (0 == BASE_PLATFORM_EMSCRIPTEN) ? 1 : 0)
#endif // GRAPHICS_CONFIG_MULTITHREADED

#ifndef GRAPHICS_CONFIG_MAX_DRAW_CALLS
#	define GRAPHICS_CONFIG_MAX_DRAW_CALLS ( (64<<10)-1)
#endif // GRAPHICS_CONFIG_MAX_DRAW_CALLS

#ifndef GRAPHICS_CONFIG_MAX_BLIT_ITEMS
#	define GRAPHICS_CONFIG_MAX_BLIT_ITEMS (1<<10)
#endif // GRAPHICS_CONFIG_MAX_BLIT_ITEMS

#ifndef GRAPHICS_CONFIG_MAX_MATRIX_CACHE
#	define GRAPHICS_CONFIG_MAX_MATRIX_CACHE (GRAPHICS_CONFIG_MAX_DRAW_CALLS+1)
#endif // GRAPHICS_CONFIG_MAX_MATRIX_CACHE

#ifndef GRAPHICS_CONFIG_MAX_RECT_CACHE
#	define GRAPHICS_CONFIG_MAX_RECT_CACHE (4<<10)
#endif //  GRAPHICS_CONFIG_MAX_RECT_CACHE

#ifndef GRAPHICS_CONFIG_SORT_KEY_NUM_BITS_DEPTH
#	define GRAPHICS_CONFIG_SORT_KEY_NUM_BITS_DEPTH 32
#endif // GRAPHICS_CONFIG_SORT_KEY_NUM_BITS_DEPTH

#ifndef GRAPHICS_CONFIG_SORT_KEY_NUM_BITS_SEQ
#	define GRAPHICS_CONFIG_SORT_KEY_NUM_BITS_SEQ 20
#endif // GRAPHICS_CONFIG_SORT_KEY_NUM_BITS_SEQ

#ifndef GRAPHICS_CONFIG_SORT_KEY_NUM_BITS_PROGRAM
#	define GRAPHICS_CONFIG_SORT_KEY_NUM_BITS_PROGRAM 9
#endif // GRAPHICS_CONFIG_SORT_KEY_NUM_BITS_PROGRAM

// Cannot be configured via compiler options.
#define GRAPHICS_CONFIG_MAX_PROGRAMS (1<<GRAPHICS_CONFIG_SORT_KEY_NUM_BITS_PROGRAM)
BASE_STATIC_ASSERT(base::isPowerOf2(GRAPHICS_CONFIG_MAX_PROGRAMS), "GRAPHICS_CONFIG_MAX_PROGRAMS must be power of 2.");

#ifndef GRAPHICS_CONFIG_MAX_VIEWS
#	define GRAPHICS_CONFIG_MAX_VIEWS 256
#endif // GRAPHICS_CONFIG_MAX_VIEWS
BASE_STATIC_ASSERT(base::isPowerOf2(GRAPHICS_CONFIG_MAX_VIEWS), "GRAPHICS_CONFIG_MAX_VIEWS must be power of 2.");

#define GRAPHICS_CONFIG_MAX_VIEW_NAME_RESERVED 6

#ifndef GRAPHICS_CONFIG_MAX_VIEW_NAME
#	define GRAPHICS_CONFIG_MAX_VIEW_NAME 256
#endif // GRAPHICS_CONFIG_MAX_VIEW_NAME

#ifndef GRAPHICS_CONFIG_MAX_VERTEX_LAYOUTS
#	define GRAPHICS_CONFIG_MAX_VERTEX_LAYOUTS 64
#endif // GRAPHICS_CONFIG_MAX_VERTEX_LAYOUTS

#ifndef GRAPHICS_CONFIG_MAX_INDEX_BUFFERS
#	define GRAPHICS_CONFIG_MAX_INDEX_BUFFERS (4<<10)
#endif // GRAPHICS_CONFIG_MAX_INDEX_BUFFERS

#ifndef GRAPHICS_CONFIG_MAX_VERTEX_BUFFERS
#	define GRAPHICS_CONFIG_MAX_VERTEX_BUFFERS (4<<10)
#endif // GRAPHICS_CONFIG_MAX_VERTEX_BUFFERS

#ifndef GRAPHICS_CONFIG_MAX_VERTEX_STREAMS
#	define GRAPHICS_CONFIG_MAX_VERTEX_STREAMS 4
#endif // GRAPHICS_CONFIG_MAX_VERTEX_STREAMS

#ifndef GRAPHICS_CONFIG_MAX_DYNAMIC_INDEX_BUFFERS
#	define GRAPHICS_CONFIG_MAX_DYNAMIC_INDEX_BUFFERS (4<<10)
#endif // GRAPHICS_CONFIG_MAX_DYNAMIC_INDEX_BUFFERS

#ifndef GRAPHICS_CONFIG_MAX_DYNAMIC_VERTEX_BUFFERS
#	define GRAPHICS_CONFIG_MAX_DYNAMIC_VERTEX_BUFFERS (4<<10)
#endif // GRAPHICS_CONFIG_MAX_DYNAMIC_VERTEX_BUFFERS

#ifndef GRAPHICS_CONFIG_DYNAMIC_INDEX_BUFFER_SIZE
#	define GRAPHICS_CONFIG_DYNAMIC_INDEX_BUFFER_SIZE (1<<20)
#endif // GRAPHICS_CONFIG_DYNAMIC_INDEX_BUFFER_SIZE

#ifndef GRAPHICS_CONFIG_DYNAMIC_VERTEX_BUFFER_SIZE
#	define GRAPHICS_CONFIG_DYNAMIC_VERTEX_BUFFER_SIZE (3<<20)
#endif // GRAPHICS_CONFIG_DYNAMIC_VERTEX_BUFFER_SIZE

#ifndef GRAPHICS_CONFIG_MAX_SHADERS
#	define GRAPHICS_CONFIG_MAX_SHADERS 512
#endif // GRAPHICS_CONFIG_MAX_FRAGMENT_SHADERS

#ifndef GRAPHICS_CONFIG_MAX_TEXTURES
#	define GRAPHICS_CONFIG_MAX_TEXTURES (4<<10)
#endif // GRAPHICS_CONFIG_MAX_TEXTURES

#ifndef GRAPHICS_CONFIG_MAX_TEXTURE_SAMPLERS
#	define GRAPHICS_CONFIG_MAX_TEXTURE_SAMPLERS 16
#endif // GRAPHICS_CONFIG_MAX_TEXTURE_SAMPLERS

#ifndef GRAPHICS_CONFIG_MAX_FRAME_BUFFERS
#	define GRAPHICS_CONFIG_MAX_FRAME_BUFFERS 128
#endif // GRAPHICS_CONFIG_MAX_FRAME_BUFFERS

#ifndef GRAPHICS_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS
#	define GRAPHICS_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS 8
#endif // GRAPHICS_CONFIG_MAX_FRAME_BUFFER_ATTACHMENTS

#ifndef GRAPHICS_CONFIG_MAX_UNIFORMS
#	define GRAPHICS_CONFIG_MAX_UNIFORMS 512
#endif // GRAPHICS_CONFIG_MAX_UNIFORMS

#ifndef GRAPHICS_CONFIG_MAX_OCCLUSION_QUERIES
#	define GRAPHICS_CONFIG_MAX_OCCLUSION_QUERIES 256
#endif // GRAPHICS_CONFIG_MAX_OCCLUSION_QUERIES

#ifndef GRAPHICS_CONFIG_MIN_RESOURCE_COMMAND_BUFFER_SIZE
#	define GRAPHICS_CONFIG_MIN_RESOURCE_COMMAND_BUFFER_SIZE (64<<10)
#endif // GRAPHICS_CONFIG_MIN_RESOURCE_COMMAND_BUFFER_SIZE

#ifndef GRAPHICS_CONFIG_TRANSIENT_VERTEX_BUFFER_SIZE
#	define GRAPHICS_CONFIG_TRANSIENT_VERTEX_BUFFER_SIZE (6<<20)
#endif // GRAPHICS_CONFIG_TRANSIENT_VERTEX_BUFFER_SIZE

#ifndef GRAPHICS_CONFIG_TRANSIENT_INDEX_BUFFER_SIZE
#	define GRAPHICS_CONFIG_TRANSIENT_INDEX_BUFFER_SIZE (2<<20)
#endif // GRAPHICS_CONFIG_TRANSIENT_INDEX_BUFFER_SIZE

#ifndef GRAPHICS_CONFIG_MAX_INSTANCE_DATA_COUNT
#	define GRAPHICS_CONFIG_MAX_INSTANCE_DATA_COUNT 5
#endif // GRAPHICS_CONFIG_MAX_INSTANCE_DATA_COUNT

#ifndef GRAPHICS_CONFIG_MAX_COLOR_PALETTE
#	define GRAPHICS_CONFIG_MAX_COLOR_PALETTE 16
#endif // GRAPHICS_CONFIG_MAX_COLOR_PALETTE

#define GRAPHICS_CONFIG_DRAW_INDIRECT_STRIDE 32

#ifndef GRAPHICS_CONFIG_PROFILER
#	define GRAPHICS_CONFIG_PROFILER 0
#endif // GRAPHICS_CONFIG_PROFILER

#ifndef GRAPHICS_CONFIG_RENDERDOC_LOG_FILEPATH
#	define GRAPHICS_CONFIG_RENDERDOC_LOG_FILEPATH "temp/graphics"
#endif // GRAPHICS_CONFIG_RENDERDOC_LOG_FILEPATH

#ifndef GRAPHICS_CONFIG_RENDERDOC_CAPTURE_KEYS
#	define GRAPHICS_CONFIG_RENDERDOC_CAPTURE_KEYS { eRENDERDOC_Key_F11 }
#endif // GRAPHICS_CONFIG_RENDERDOC_CAPTURE_KEYS

#ifndef GRAPHICS_CONFIG_API_SEMAPHORE_TIMEOUT
#	define GRAPHICS_CONFIG_API_SEMAPHORE_TIMEOUT (5000)
#endif // GRAPHICS_CONFIG_API_SEMAPHORE_TIMEOUT

#ifndef GRAPHICS_CONFIG_MIP_LOD_BIAS
#	define GRAPHICS_CONFIG_MIP_LOD_BIAS 0
#endif // GRAPHICS_CONFIG_MIP_LOD_BIAS

#ifndef GRAPHICS_CONFIG_DEFAULT_MAX_ENCODERS
#	define GRAPHICS_CONFIG_DEFAULT_MAX_ENCODERS ( (0 != GRAPHICS_CONFIG_MULTITHREADED) ? 8 : 1)
#endif // GRAPHICS_CONFIG_DEFAULT_MAX_ENCODERS

#ifndef GRAPHICS_CONFIG_MAX_BACK_BUFFERS
#	define GRAPHICS_CONFIG_MAX_BACK_BUFFERS 4
#endif // GRAPHICS_CONFIG_MAX_BACK_BUFFERS

#ifndef GRAPHICS_CONFIG_MAX_FRAME_LATENCY
#	define GRAPHICS_CONFIG_MAX_FRAME_LATENCY 3
#endif // GRAPHICS_CONFIG_MAX_FRAME_LATENCY

#ifndef GRAPHICS_CONFIG_PREFER_DISCRETE_GPU
// On laptops with integrated and discrete GPU, prefer selection of discrete GPU.
// nVidia and AMD, on Windows only.
#	define GRAPHICS_CONFIG_PREFER_DISCRETE_GPU BASE_PLATFORM_WINDOWS
#endif // GRAPHICS_CONFIG_PREFER_DISCRETE_GPU

#ifndef GRAPHICS_CONFIG_MAX_SCREENSHOTS
#	define GRAPHICS_CONFIG_MAX_SCREENSHOTS 4
#endif // GRAPHICS_CONFIG_MAX_SCREENSHOTS

#ifndef GRAPHICS_CONFIG_ENCODER_API_ONLY
#	define GRAPHICS_CONFIG_ENCODER_API_ONLY 0
#endif // GRAPHICS_CONFIG_ENCODER_API_ONLY

#endif // GRAPHICS_CONFIG_H_HEADER_GUARD
