#include "mrender/handler/shader.hpp"
#include "mrender/core/file_ops.hpp"

namespace mrender {

void ShaderImplementation::loadProgram(char const* fileName, char const* filePath)
{
    mFileName = fileName;
    mFilePath = filePath;
	const std::string vertexPath = std::string(filePath) + "/" + std::string(fileName) + "-vert.bin";
	const std::string fragmentPath = std::string(filePath) + "/" + std::string(fileName) + "-frag.bin";

    std::string vshader;
    if (!mrender::read_file(vertexPath, vshader)) 
    {}

    std::string fshader;
    if (!mrender::read_file(fragmentPath, fshader)) 
    {}

	bgfx::ShaderHandle vsh = createShader(vshader, "vshader");
	bgfx::ShaderHandle fsh = createShader(fshader, "fshader");

	mHandle = bgfx::createProgram(vsh, fsh, true);

    bgfx::destroy(vsh);
    bgfx::destroy(fsh);
}

void ShaderImplementation::reloadProgram()
{
    bgfx::destroy(mHandle);
    loadProgram(mFileName, mFilePath);
}

bgfx::ShaderHandle ShaderImplementation::createShader(const std::string& shader, const char* name)
{
    const bgfx::Memory* mem = bgfx::copy(shader.data(), shader.size());
    const bgfx::ShaderHandle handle = bgfx::createShader(mem);
    bgfx::setName(handle, name);
    return handle;
}

}	// namespace mrender