#include "mrender/renderers/techniques/testing.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

namespace mrender {

void TestingTechnique::initialize(RenderContext& context)
{
    // @todo should this be here?
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6060FFFF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, context.getSettings().mResolutionWidth, context.getSettings().mResolutionHeight);
}

void TestingTechnique::render()
{
    bgfx::touch(0);
    bgfx::dbgTextClear();

    bgfx::dbgTextPrintf(0, 0, 0x0f, "Press F1 to toggle stats.");
    bgfx::dbgTextPrintf(
        0, 1, 0x0f,
        "Color can be changed with ANSI "
        "\x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m "
        "code too.");
    bgfx::dbgTextPrintf(
        80, 1, 0x0f,
        "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; "
        "5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
    bgfx::dbgTextPrintf(
        80, 2, 0x0f,
        "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    "
        "\x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");
    const bgfx::Stats* stats = bgfx::getStats();
    bgfx::dbgTextPrintf(
        0, 2, 0x0f,
        "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters.",
        stats->width, stats->height, stats->textWidth, stats->textHeight);

    bgfx::setDebug(true ? BGFX_DEBUG_STATS : BGFX_DEBUG_TEXT);

    bgfx::frame();
}

}   // namespace mrender