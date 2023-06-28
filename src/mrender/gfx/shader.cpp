#include "mrender/gfx/shader.hpp"
#include "mrender/utils/file_ops.hpp"

namespace mrender {

void ShaderImplementation::loadProgram(char const* fileName, char const* filePath)
{
    mFileName = fileName;
    mFilePath = filePath;
	const std::string vertexPath = std::string(mFilePath) + "/" + std::string(mFileName) + "-vert.bin";
	const std::string fragmentPath = std::string(mFilePath) + "/" + std::string(mFileName) + "-frag.bin";

    std::string vshader;
    if (!mrender::read_file(vertexPath, vshader))
    {
        std::cout << "Invalid path when loading shader" << std::endl;
        return;
    }
    bgfx::ShaderHandle vsh = BGFX_INVALID_HANDLE;
    vsh = createShader(vshader, "vshader");
    if (!bgfx::isValid(vsh))
    {
        std::cout << "Failed to create shader" << std::endl;
        return;
    }
 
    std::string fshader;
    if (!mrender::read_file(fragmentPath, fshader))
    {
        std::cout << "Invalid path when loading shader" << std::endl;
        return;
    }
    bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
    fsh = createShader(fshader, "fshader");
    if (!bgfx::isValid(fsh))
    {
        std::cout << "Failed to create shader" << std::endl;
        return;
    }
    
    mHandle = bgfx::createProgram(vsh, fsh, true);
    if (!bgfx::isValid(mHandle))
    {
        std::cout << "Invalid Shader handle: " << fileName << std::endl;
    }

    // Store all fragment uniforms
    uint16_t uniformNum = bgfx::getShaderUniforms(fsh, nullptr, 0);
    bgfx::UniformHandle* uniforms = new bgfx::UniformHandle[uniformNum];
    bgfx::getShaderUniforms(fsh, uniforms, uniformNum);
    
    uint8_t unit = 0;
    for (uint8_t i = 0; i < uniformNum; i++)
    {
        bgfx::UniformInfo info;
        bgfx::getUniformInfo(uniforms[i], info);
        std::string name = info.name;
        
        mUniformHandles[name] = uniforms[i];

        printf("Created uniform %s in shader %s number %u\n", name, fileName, i);
    }
}

void ShaderImplementation::reloadProgram()
{
    if (bgfx::isValid(mHandle))
    {
        bgfx::destroy(mHandle);
    }
    loadProgram(mFileName, mFilePath);
}

bgfx::ShaderHandle ShaderImplementation::createShader(const std::string& shader, const char* name)
{
    const bgfx::Memory* mem = bgfx::copy(shader.data(), static_cast<uint32_t>(shader.size()));
    const bgfx::ShaderHandle handle = bgfx::createShader(mem);
    bgfx::setName(handle, name);
    return handle;
}

}	// namespace mrender