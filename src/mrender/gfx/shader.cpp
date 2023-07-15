#include "mrender/gfx/shader.hpp"
#include "mrender/utils/file_ops.hpp"

#include <filesystem>

namespace mrender {

ShaderImplementation::ShaderImplementation()
    : mHandle(BGFX_INVALID_HANDLE)
{
}

ShaderImplementation::~ShaderImplementation()
{
    bgfx::destroy(mHandle);
}

void ShaderImplementation::loadProgram(const std::string& vertexPath, const std::string& fragmentPath)
{
    mVertexFilePath = vertexPath;
    mFragmentFilePath = fragmentPath;

    std::filesystem::path filePath = std::filesystem::path(mFragmentFilePath);
    std::string filename = filePath.filename().string();
    mFileName = filename.substr(0, filename.find_last_of('-'));

    std::string vshader;
    if (!mrender::read_file(mVertexFilePath, vshader))
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
    if (!mrender::read_file(mFragmentFilePath, fshader))
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
        std::cout << "Invalid Shader handle: " << mFileName << std::endl;
    }

    // Store all fragment uniforms
    uint16_t uniformNum = bgfx::getShaderUniforms(fsh, nullptr, 0);
    bgfx::UniformHandle* uniforms = new bgfx::UniformHandle[uniformNum];
    bgfx::getShaderUniforms(fsh, uniforms, uniformNum);
    
    uint8_t unit = 0;
    for (uint8_t i = 0; i < uniformNum; i++)
    {
        if (!bgfx::isValid(uniforms[i]))
        {
            return;
        }

        bgfx::UniformInfo info;
        bgfx::getUniformInfo(uniforms[i], info);
        std::string name = info.name;
        
        if (info.type == bgfx::UniformType::Sampler)
        {
            mUniformHandles[name] = { uniforms[i], unit };
            unit++;
        }
        else
        {
            mUniformHandles[name] = { uniforms[i], 0 };
        }

        printf("Created uniform %s in shader %s number %u\n", name.data(), mFileName.data(), i);
    }
}

void ShaderImplementation::reloadProgram()
{
    if (bgfx::isValid(mHandle))
    {
        bgfx::destroy(mHandle);
    }
    loadProgram(mVertexFilePath, mFragmentFilePath);
}

bgfx::ShaderHandle ShaderImplementation::createShader(const std::string& shader, const char* name)
{
    const bgfx::Memory* mem = bgfx::copy(shader.data(), static_cast<uint32_t>(shader.size()));
    const bgfx::ShaderHandle handle = bgfx::createShader(mem);
    bgfx::setName(handle, name);
    return handle;
}

}	// namespace mrender