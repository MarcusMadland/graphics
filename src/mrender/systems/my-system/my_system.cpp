#include "mrender/systems/my-system/my_system.hpp"
#include "mrender/core/file_ops.hpp"

#include <bgfx/bgfx.h>
#include <bx/math.h>

namespace mrender {

MySystem::MySystem()
    : RenderSystem("My System")
{
}

MySystem::~MySystem()
{
}

bool MySystem::init(mrender::RenderContext& context)
{ 
    // Scene Camera
    CameraSettings cameraSettings;
    cameraSettings.width = context.getSettings().mResolutionWidth;
    cameraSettings.height = context.getSettings().mResolutionHeight;
    cameraSettings.postion[2] = -5.0f;
    mCamera = context.createCamera(cameraSettings);

    // Shader
    context.loadShader("screen", "C:/Users/marcu/Dev/my-application/mrender/shaders/screen");

    // Screen quad
    BufferLayout layout =
    { {
        { AttribType::Float, 3, Attrib::Position },
        { AttribType::Float, 2, Attrib::TexCoord0 },
    } };
    mScreenQuad = context.createGeometry(layout, quadVertices.data(), quadVertices.size() * sizeof(VertexData), quadIndices);

    return true;
}

void MySystem::render(RenderContext& context)
{
    // Shadow Pass
    context.writeToBuffer("Shadow");
    context.clear();
  
    bgfx::setState(0 );

    context.submit(context.getRenderables(), mCamera);

    // Scene Pass
    context.writeToBuffer("Scene");
    context.clear();

    bgfx::setState(0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_DEPTH_TEST_LESS
        | BGFX_STATE_MSAA
    );

    context.submit(context.getRenderables(), context.getCamera());

    // Post Proces Pass
    context.writeToBuffer("PostProcess", true);
    context.clear();

    bgfx::setState(0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A
    );

    context.setParameter("screen", "s_Shadow", context.getBuffers().at("Shadow")->getDepthBuffer());
    context.setParameter("screen", "s_Scene", context.getBuffers().at("Scene")->getColorBuffer());

    context.submit(mScreenQuad, "screen", nullptr);
}

std::vector<std::pair<std::string, std::shared_ptr<FrameBuffer>>> MySystem::getBuffers(RenderContext& context)
{
    std::vector<std::pair<std::string, std::shared_ptr<FrameBuffer>>> buffers;
    buffers.push_back({ "Shadow", context.createFrameBuffer(TextureFormat::BGRA8, true) });
    buffers.push_back({ "Scene", context.createFrameBuffer(TextureFormat::BGRA8, true) });
    buffers.push_back({ "PostProcess", context.createFrameBuffer(TextureFormat::BGRA8) });
    return buffers;
}

}   // namespace mrender