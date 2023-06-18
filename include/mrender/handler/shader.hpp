#pragma once

#include "mrender/mrender.hpp"

#include <string_view>
#include <bgfx/bgfx.h>

namespace mrender {

struct UniformData
{
	bgfx::UniformHandle mHandle = BGFX_INVALID_HANDLE;
	uint8_t unit = UINT8_MAX;
};

class ShaderImplementation : public Shader
{
	friend class RenderContextImplementation;

public:
	virtual void loadProgram(char const* fileName, char const* filePath) override;
	virtual void reloadProgram() override;

private:
	bgfx::ShaderHandle createShader(const std::string& shader, const char* name);

private:
	bgfx::ProgramHandle mHandle = BGFX_INVALID_HANDLE;
	std::unordered_map<std::string, UniformData> mUniformHandles;
	const char* mFileName = nullptr;
	const char* mFilePath = nullptr;
};

}	// namespace mrender