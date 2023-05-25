#include "mrender/systems/testing/testing.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

Testing::Testing()
{
}

Testing::~Testing()
{
}

bool Testing::init(mrender::RenderContext& context) 
{
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x202020FF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, context.getSettings().mResolutionWidth, context.getSettings().mResolutionHeight);

    return true;
}

void Testing::render(mrender::RenderContext& mrender) 
{
    bgfx::touch(0);
   
  
    const bgfx::Stats* stats = bgfx::getStats();

    // Testing text and colors
    bgfx::dbgTextClear();
    bgfx::dbgTextPrintf(2, 1, 0x0f, 
        "frametime: %u", stats->cpuTimeFrame);
 
    bgfx::dbgTextPrintf(80, 1, 0x0f,
        "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; "
        "5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
    bgfx::dbgTextPrintf(80, 2, 0x0f,
        "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    "
        "\x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");
    
    // Debug @todo make option system that works with render systems, then put an option for this?
    bgfx::setDebug(false ? BGFX_DEBUG_STATS : BGFX_DEBUG_TEXT);

    bgfx::frame();
}

}   // namespace mrender