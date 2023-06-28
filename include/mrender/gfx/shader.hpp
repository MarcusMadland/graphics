#pragma once

#include "mrender/mrender.hpp"

#include <string_view>
#include <bgfx/bgfx.h>

namespace mrender {

class ShaderImplementation : public Shader
{
	friend class RenderContextImplementation;
	friend class MaterialImplementation;

public:
	virtual void loadProgram(char const* fileName, char const* filePath) override;
	virtual void reloadProgram() override;

private:
	bgfx::ShaderHandle createShader(const std::string& shader, const char* name);

private:
	bgfx::ProgramHandle mHandle = BGFX_INVALID_HANDLE;
	std::unordered_map<std::string, bgfx::UniformHandle> mUniformHandles;
	const char* mFileName = nullptr;
	const char* mFilePath = nullptr;
};

}	// namespace mrender