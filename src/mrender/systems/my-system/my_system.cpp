#include "mrender/systems/my-system/my_system.hpp"
#include "mrender/core/file_ops.hpp"

#include <bgfx/bgfx.h>
#include <bx/math.h>

namespace mrender {

static PosColorVertex cubeVertices[] =
{
    {-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},
    {-1.0f, -1.0f, 1.0f, 0xff00ff00},  {1.0f, -1.0f, 1.0f, 0xff00ffff},
    {-1.0f, 1.0f, -1.0f, 0xffff0000},  {1.0f, 1.0f, -1.0f, 0xffff00ff},
    {-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
};

static const uint16_t cubeTriList[] =
{
    0, 1, 2, 1, 3, 2, 4, 6, 5, 5, 6, 7, 0, 2, 4, 4, 2, 6,
    1, 5, 3, 5, 7, 3, 0, 4, 1, 4, 5, 1, 2, 3, 6, 6, 3, 7,
};

static bgfx::ShaderHandle create_shader(
    const std::string& shader, const char* name)
{
    const bgfx::Memory* mem = bgfx::copy(shader.data(), shader.size());
    const bgfx::ShaderHandle handle = bgfx::createShader(mem);
    bgfx::setName(handle, name);
    return handle;
}

MySystem::MySystem()
    : RenderSystem("My System")
{
}

MySystem::~MySystem()
{
}

bool MySystem::init(const mrender::RenderContext& context)
{
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDFF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, context.getSettings().mResolutionWidth, context.getSettings().mResolutionHeight);
    
    std::string vshader;
    if (!mrender::read_file( "C:/Users/marcu/Dev/my-application/mrender/shaders/simple/simple-vert.bin", vshader)) {
        printf("Could not find shader vertex shader (ensure shaders have been "
            "compiled).\n"
            "Run compile-shaders-<platform>.sh/bat\n");
        return 1;
    }

    std::string fshader;
    if (!mrender::read_file("C:/Users/marcu/Dev/my-application/mrender/shaders/simple/simple-frag.bin", fshader)) {
        printf("Could not find shader fragment shader (ensure shaders have "
            "been compiled).\n"
            "Run compile-shaders-<platform>.sh/bat\n");
        return 1;
    }

    bgfx::VertexLayout pos_col_vert_layout;
    pos_col_vert_layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();
    vbh = bgfx::createVertexBuffer(
        bgfx::makeRef(cubeVertices, sizeof(cubeVertices)),
        pos_col_vert_layout);
    ibh = bgfx::createIndexBuffer(
        bgfx::makeRef(cubeTriList, sizeof(cubeTriList)));


    bgfx::ShaderHandle vsh = create_shader(vshader, "vshader");
    bgfx::ShaderHandle fsh = create_shader(fshader, "fshader");
    program = bgfx::createProgram(vsh, fsh, true);
    
    return true;
}

void MySystem::render(mrender::RenderContext& context)
{
    float cam_rotation[16];
    bx::mtxRotateXYZ(cam_rotation, cam_pitch, cam_yaw, 0.0f);

    float cam_translation[16];
    bx::mtxTranslate(cam_translation, 0.0f, 0.0f, -5.0f);

    float cam_transform[16];
    bx::mtxMul(cam_transform, cam_translation, cam_rotation);

    float view[16];
    bx::mtxInverse(view, cam_transform);

    float proj[16];
    bx::mtxProj(
        proj, 60.0f, float(context.getSettings().mResolutionWidth) / float(context.getSettings().mResolutionHeight), 0.1f,
        100.0f, bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewTransform(0, view, proj);

    float model[16];
    bx::mtxIdentity(model);
    bgfx::setTransform(model);

    bgfx::setVertexBuffer(0, vbh);
    bgfx::setIndexBuffer(ibh);

    bgfx::submit(0, program);
}

}   // namespace mrender