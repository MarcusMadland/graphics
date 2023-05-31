#include "mrender/handler/shader.hpp"
#include "mrender/core/file_ops.hpp"

#include <shaderc/shaderc.h>

namespace mrender {

void ShaderImplementation::loadProgram(char const* fileName, char const* filePath)
{
    mFileName = fileName;
    mFilePath = filePath;
	const std::string vertexPath = std::string(filePath) + "/" + std::string(fileName) + "-vert.sc";
	const std::string fragmentPath = std::string(filePath) + "/" + std::string(fileName) + "-frag.sc";

    // Determine renderer extension
    bgfx::RendererType::Enum renderer = bgfx::getRendererType();
    const char* rendererExt;
    switch (renderer)
    {
    case bgfx::RendererType::Direct3D12:
        rendererExt = "d3d12";
        break;

    case bgfx::RendererType::Direct3D11:
        rendererExt = "d3d11";
        break;

    case bgfx::RendererType::Direct3D9:
        rendererExt = "d3d9";
        break;

    case bgfx::RendererType::Metal:
        rendererExt = "mtl";
        break;

    default:
        rendererExt = "gl";
        break;
    }

    // Output from any shader compilations. Errors, etc.
    char shader_output[UINT16_MAX];
    uint16_t shader_output_size = 0;

    // Vertex Shader
    const std::string vertex_compiled_path = std::string(filePath) + "/" + rendererExt + "/" + "-vert.bin";
    const char* cachedVertexPath = vertex_compiled_path.c_str();
    if (true)
    {
        switch (renderer)
        {
        case bgfx::RendererType::Direct3D12:
            compileShader(0, vertexPath.c_str(), cachedVertexPath, "v", "windows", "vs_4_0", NULL, filePath, filePath, shader_output, shader_output_size);
            break;

        case bgfx::RendererType::Direct3D11:
            compileShader(0, vertexPath.c_str(), cachedVertexPath, "v", "windows", "vs_5_0", NULL, filePath, filePath, shader_output, shader_output_size);
            break;

        case bgfx::RendererType::Direct3D9:
            compileShader(0, vertexPath.c_str(), cachedVertexPath, "v", "windows", "vs_3_0", NULL, filePath, filePath, shader_output, shader_output_size);
            break;

        case bgfx::RendererType::Metal:
            compileShader(0, vertexPath.c_str(), cachedVertexPath, "v", "osx", "metal", NULL, filePath, filePath, shader_output, shader_output_size);
            break;

        default:
            compileShader(0, vertexPath.c_str(), cachedVertexPath, "v", "osx", "120", NULL, filePath, filePath, shader_output, shader_output_size);
            break;
        }
    }

    std::string vshader;
    if (!mrender::read_file(shader_output, vshader))
    {
        return;
    }
    bgfx::ShaderHandle vsh = createShader(vshader, "vshader");

    // Fragment Shader
    const std::string fragment_compiled_path = std::string(filePath) + "/" + rendererExt + "/" + "-frag.bin";
    const char* cachedFragmentPath = fragment_compiled_path.c_str();
    if (true)
    {
        switch (renderer)
        {
        case bgfx::RendererType::Direct3D12:
            compileShader(0, fragmentPath.c_str(), cachedFragmentPath, "f", "windows", "ps_4_0", NULL, filePath, filePath, shader_output, shader_output_size);
            break;

        case bgfx::RendererType::Direct3D11:
            compileShader(0, fragmentPath.c_str(), cachedFragmentPath, "f", "windows", "ps_5_0", NULL, filePath, filePath, shader_output, shader_output_size);
            break;

        case bgfx::RendererType::Direct3D9:
            compileShader(0, fragmentPath.c_str(), cachedFragmentPath, "f", "windows", "ps_3_0", NULL, filePath, filePath, shader_output, shader_output_size);
            break;

        case bgfx::RendererType::Metal:
            compileShader(0, fragmentPath.c_str(), cachedFragmentPath, "f", "osx", "metal", NULL, filePath, filePath, shader_output, shader_output_size);
            break;

        default:
            compileShader(0, fragmentPath.c_str(), cachedFragmentPath, "f", "osx", "120", NULL, filePath, filePath, shader_output, shader_output_size);
            break;
        }
    }

    std::string fshader;
    if (!mrender::read_file(shader_output, fshader))
    {
        return;
    }
    bgfx::ShaderHandle fsh = createShader(fshader, "fshader");

    // Create program
    mHandle = bgfx::createProgram(vsh, fsh, true);
   
    // Cleanup
    bgfx::destroy(vsh);
    bgfx::destroy(fsh);
}

void ShaderImplementation::reloadProgram()
{
    bgfx::destroy(mHandle);
    loadProgram(mFileName, mFilePath);
}

void ShaderImplementation::compileShader(uint64_t _flags, const char* _filePath, const char* _outFilePath, const char* _type, const char* _platform, const char* _profile, const char* _bin2c, const char* _includeDir, const char* _varyingdef, char* _outputText, uint16_t& _outputSize)
{
    const char* argv[16];
    int argc = 0;

    // -f <file path>                Input file path.
    argv[argc] = "-f";
    argv[argc + 1] = _filePath;
    argc += 2;

    // -o <file path>                Output file path.
    argv[argc] = "-o";
    argv[argc + 1] = _outFilePath;
    argc += 2;

    // --platform <platform>     Target platform.
    argv[argc] = "--platform";
    argv[argc + 1] = _platform;
    argc += 2;

    // --type <type>             Shader type (vertex, fragment)
    argv[argc] = "--type";
    argv[argc + 1] = _type;
    argc += 2;

    // -i <include path>             Include path (for multiple paths use semicolon).
    if (_includeDir)
    {
        argv[argc] = "-i";
        argv[argc + 1] = _includeDir;
        argc += 2;
    }

    // --bin2c <file path>       Generate C header file.
    if (_bin2c)
    {
        argv[argc] = "--bin2c";
        argv[argc + 1] = _bin2c;
        argc += 2;
    }

    // --varyingdef <file path>  Path to varying.def.sc file.
    if (_varyingdef)
    {
        argv[argc] = "--varyingdef";
        argv[argc + 1] = _varyingdef;
        argc += 2;
    }

    // -p, --profile <profile>       Shader model (f.e. ps_3_0).
    if (_profile)
    {
        argv[argc] = "-p";
        argv[argc + 1] = _profile;
        argc += 2;
    }

    //bgfx::Options option;
    //int result = bgfx::compileGLSLShader(option, 0, "", nullptr);
    //std::cout << "Result of compiling shader: " << result << std::endl;
}


bgfx::ShaderHandle ShaderImplementation::createShader(const std::string& shader, const char* name)
{
    const bgfx::Memory* mem = bgfx::copy(shader.data(), shader.size());
    const bgfx::ShaderHandle handle = bgfx::createShader(mem);
    bgfx::setName(handle, name);
    return handle;
}

}	// namespace mrender