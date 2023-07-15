#pragma once

#include "mrender/mrender.hpp"

#include <string_view>
#include <bgfx/bgfx.h>

namespace mrender {

class ShaderImplementation : public Shader
{
	friend class GfxContextImplementation;
	friend class MaterialImplementation;

public:
	ShaderImplementation();
	~ShaderImplementation();

	virtual void loadProgram(const std::string& vertexPath, const std::string& fragmentPath);
	virtual void reloadProgram();

private:
	bgfx::ShaderHandle createShader(const std::string& shader, const char* name);

private:
	bgfx::ProgramHandle mHandle;
	std::unordered_map<std::string, std::pair<bgfx::UniformHandle, uint8_t>> mUniformHandles;
	std::string mFileName;
	std::string mVertexFilePath;
	std::string mFragmentFilePath;
};

}	// namespace mrender