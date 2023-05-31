#pragma once

#include "mrender/mrender.hpp"

#include <string_view>
#include <bgfx/bgfx.h>

namespace mrender {

class ShaderImplementation : public Shader
{
	friend class RenderContextImplementation;

public:
	virtual void loadProgram(char const* fileName, char const* filePath) override;
	virtual void reloadProgram() override;

private:
	void compileShader(uint64_t _flags, const char* _filePath, const char* _outFilePath, const char* _type, const char* _platform, const char* _profile, const char* _bin2c, const char* _includeDir, const char* _varyingdef, char* _outputText, uint16_t& _outputSize);

	bgfx::ShaderHandle createShader(const std::string& shader, const char* name);

private:
	bgfx::ProgramHandle mHandle = BGFX_INVALID_HANDLE;
	const char* mFileName = nullptr;
	const char* mFilePath = nullptr;
};

}	// namespace mrender