/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#ifndef GRAPHICS_EMBEDDED_SHADER_H_HEADER_GUARD
#define GRAPHICS_EMBEDDED_SHADER_H_HEADER_GUARD

#include <base/base.h>
#include "graphics.h"

#define GRAPHICS_EMBEDDED_SHADER_DXBC(...)
#define GRAPHICS_EMBEDDED_SHADER_DX9BC(...)
#define GRAPHICS_EMBEDDED_SHADER_PSSL(...)
#define GRAPHICS_EMBEDDED_SHADER_ESSL(...)
#define GRAPHICS_EMBEDDED_SHADER_GLSL(...)
#define GRAPHICS_EMBEDDED_SHADER_METAL(...)
#define GRAPHICS_EMBEDDED_SHADER_NVN(...)
#define GRAPHICS_EMBEDDED_SHADER_SPIRV(...)

#define GRAPHICS_PLATFORM_SUPPORTS_DX9BC (0 \
	|| BASE_PLATFORM_LINUX                \
	|| BASE_PLATFORM_WINDOWS              \
	)
#define GRAPHICS_PLATFORM_SUPPORTS_DXBC (0  \
	|| BASE_PLATFORM_LINUX                \
	|| BASE_PLATFORM_WINDOWS              \
	|| BASE_PLATFORM_WINRT                \
	|| BASE_PLATFORM_XBOXONE              \
	)
#define GRAPHICS_PLATFORM_SUPPORTS_PSSL (0  \
	|| BASE_PLATFORM_PS4                  \
	|| BASE_PLATFORM_PS5                  \
	)
#define GRAPHICS_PLATFORM_SUPPORTS_ESSL (0  \
	|| BASE_PLATFORM_ANDROID              \
	|| BASE_PLATFORM_EMSCRIPTEN           \
	|| BASE_PLATFORM_IOS                  \
	|| BASE_PLATFORM_LINUX                \
	|| BASE_PLATFORM_OSX                  \
	|| BASE_PLATFORM_RPI                  \
	|| BASE_PLATFORM_WINDOWS              \
	)
#define GRAPHICS_PLATFORM_SUPPORTS_GLSL (0  \
	|| BASE_PLATFORM_BSD                  \
	|| BASE_PLATFORM_LINUX                \
	|| BASE_PLATFORM_OSX                  \
	|| BASE_PLATFORM_WINDOWS              \
	)
#define GRAPHICS_PLATFORM_SUPPORTS_METAL (0 \
	|| BASE_PLATFORM_IOS                  \
	|| BASE_PLATFORM_OSX                  \
	)
#define GRAPHICS_PLATFORM_SUPPORTS_NVN (0   \
	|| BASE_PLATFORM_NX                   \
	)
#define GRAPHICS_PLATFORM_SUPPORTS_SPIRV (0 \
	|| BASE_PLATFORM_ANDROID              \
	|| BASE_PLATFORM_EMSCRIPTEN           \
	|| BASE_PLATFORM_LINUX                \
	|| BASE_PLATFORM_WINDOWS              \
	|| BASE_PLATFORM_OSX                  \
	)

#if GRAPHICS_PLATFORM_SUPPORTS_DX9BC
#	undef  GRAPHICS_EMBEDDED_SHADER_DX9BC
#	define GRAPHICS_EMBEDDED_SHADER_DX9BC(_renderer, _name) \
	{ _renderer, BASE_CONCATENATE(_name, _dx9 ), BASE_COUNTOF(BASE_CONCATENATE(_name, _dx9 ) ) },
#endif // GRAPHICS_PLATFORM_SUPPORTS_DX9BC

#if GRAPHICS_PLATFORM_SUPPORTS_DXBC
#	undef  GRAPHICS_EMBEDDED_SHADER_DXBC
#	define GRAPHICS_EMBEDDED_SHADER_DXBC(_renderer, _name) \
	{ _renderer, BASE_CONCATENATE(_name, _dx11), BASE_COUNTOF(BASE_CONCATENATE(_name, _dx11) ) },
#endif // GRAPHICS_PLATFORM_SUPPORTS_DXBC

#if GRAPHICS_PLATFORM_SUPPORTS_PSSL
#	undef  GRAPHICS_EMBEDDED_SHADER_PSSL
#	define GRAPHICS_EMBEDDED_SHADER_PSSL(_renderer, _name) \
	{ _renderer, BASE_CONCATENATE(_name, _pssl), BASE_CONCATENATE(_name, _pssl_size) },
#endif // GRAPHICS_PLATFORM_SUPPORTS_PSSL

#if GRAPHICS_PLATFORM_SUPPORTS_ESSL
#	undef  GRAPHICS_EMBEDDED_SHADER_ESSL
#	define GRAPHICS_EMBEDDED_SHADER_ESSL(_renderer, _name) \
	{ _renderer, BASE_CONCATENATE(_name, _essl), BASE_COUNTOF(BASE_CONCATENATE(_name, _essl) ) },
#endif // GRAPHICS_PLATFORM_SUPPORTS_ESSL

#if GRAPHICS_PLATFORM_SUPPORTS_GLSL
#	undef  GRAPHICS_EMBEDDED_SHADER_GLSL
#	define GRAPHICS_EMBEDDED_SHADER_GLSL(_renderer, _name) \
	{ _renderer, BASE_CONCATENATE(_name, _glsl), BASE_COUNTOF(BASE_CONCATENATE(_name, _glsl) ) },
#endif // GRAPHICS_PLATFORM_SUPPORTS_GLSL

#if GRAPHICS_PLATFORM_SUPPORTS_SPIRV
#	undef  GRAPHICS_EMBEDDED_SHADER_SPIRV
#	define GRAPHICS_EMBEDDED_SHADER_SPIRV(_renderer, _name) \
	{ _renderer, BASE_CONCATENATE(_name, _spv), BASE_COUNTOF(BASE_CONCATENATE(_name, _spv) ) },
#endif // GRAPHICS_PLATFORM_SUPPORTS_SPIRV

#if GRAPHICS_PLATFORM_SUPPORTS_METAL
#	undef  GRAPHICS_EMBEDDED_SHADER_METAL
#	define GRAPHICS_EMBEDDED_SHADER_METAL(_renderer, _name) \
	{ _renderer, BASE_CONCATENATE(_name, _mtl), BASE_COUNTOF(BASE_CONCATENATE(_name, _mtl) ) },
#endif // GRAPHICS_PLATFORM_SUPPORTS_METAL

#define GRAPHICS_EMBEDDED_SHADER(_name)                                                        \
	{                                                                                      \
		#_name,                                                                            \
		{                                                                                  \
			GRAPHICS_EMBEDDED_SHADER_PSSL (graphics::RendererType::Agc,        _name)              \
			GRAPHICS_EMBEDDED_SHADER_DX9BC(graphics::RendererType::Direct3D9,  _name)              \
			GRAPHICS_EMBEDDED_SHADER_DXBC (graphics::RendererType::Direct3D11, _name)              \
			GRAPHICS_EMBEDDED_SHADER_DXBC (graphics::RendererType::Direct3D12, _name)              \
			GRAPHICS_EMBEDDED_SHADER_PSSL (graphics::RendererType::Gnm,        _name)              \
			GRAPHICS_EMBEDDED_SHADER_METAL(graphics::RendererType::Metal,      _name)              \
			GRAPHICS_EMBEDDED_SHADER_NVN  (graphics::RendererType::Nvn,        _name)              \
			GRAPHICS_EMBEDDED_SHADER_ESSL (graphics::RendererType::OpenGLES,   _name)              \
			GRAPHICS_EMBEDDED_SHADER_GLSL (graphics::RendererType::OpenGL,     _name)              \
			GRAPHICS_EMBEDDED_SHADER_SPIRV(graphics::RendererType::Vulkan,     _name)              \
			GRAPHICS_EMBEDDED_SHADER_SPIRV(graphics::RendererType::WebGPU,     _name)              \
			{ graphics::RendererType::Noop,  (const uint8_t*)"VSH\x5\x0\x0\x0\x0\x0\x0", 10 }, \
			{ graphics::RendererType::Count, NULL, 0 }                                         \
		}                                                                                  \
	}

#define GRAPHICS_EMBEDDED_SHADER_END()                 \
	{                                              \
		NULL,                                      \
		{                                          \
			{ graphics::RendererType::Count, NULL, 0 } \
		}                                          \
	}

namespace graphics
{
	struct EmbeddedShader
	{
		struct Data
		{
			RendererType::Enum type;
			const uint8_t* data;
			uint32_t size;
		};

		const char* name;
		Data data[RendererType::Count];
	};

	/// Create shader from embedded shader data.
	///
	/// @param[in] _es Pointer to `GRAPHICS_EMBEDDED_SHADER` data.
	/// @param[in] _type Renderer backend type. See: `graphics::RendererType`
	/// @param[in] _name Shader name.
	/// @returns Shader handle.
	///
	ShaderHandle createEmbeddedShader(
		  const graphics::EmbeddedShader* _es
		, RendererType::Enum _type
		, const char* _name
		);

} // namespace graphics

#endif // GRAPHICS_EMBEDDED_SHADER_H_HEADER_GUARD
