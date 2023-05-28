#include "mrender/systems/my-system2/my_system2.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

MySystem2::MySystem2()
    : RenderSystem("My System 2")
{
}

MySystem2::~MySystem2()
{
}

bool MySystem2::init(mrender::RenderContext& context)
{
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x202020FF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, context.getSettings().mResolutionWidth, context.getSettings().mResolutionHeight);

    return true;
}

void MySystem2::render(mrender::RenderContext& context)
{
    bgfx::dbgTextPrintf(80, 19, 0x0f, "MySystem2");

    bgfx::dbgTextPrintf(80, 20, 0x0f,
        "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; "
        "5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
    bgfx::dbgTextPrintf(80, 21, 0x0f,
        "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    "
        "\x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");

    // Debug @todo make option system that works with render systems, then put an option for this?
    

}

}   // namespace mrender