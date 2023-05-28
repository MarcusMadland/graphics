#pragma once

#include <bgfx/bgfx.h>
#include <string_view>

namespace mrender {

class Shader
{
public:
	void loadProgram(char const* fileName, char const* filePath);
	void reloadProgram();

	[[nodiscard]] const std::string_view& getName() const { return mFileName; }

private:
	bgfx::ShaderHandle createShader(const std::string& shader, const char* name);

private:
	bgfx::ProgramHandle mHandle = BGFX_INVALID_HANDLE;

	const char* mFileName = nullptr;
	const char* mFilePath = nullptr;
};

}	// namespace mrender