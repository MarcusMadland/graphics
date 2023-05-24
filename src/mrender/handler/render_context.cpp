#include "mrender/handler/render_context.hpp"
#include "mrender/core/file_ops.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>
#include <string>
#include <cassert>

namespace mrender {

void RenderContext::initialize(const RenderSettings& settings)
{
    if (settings.mNativeWindow == nullptr)
    {
        return; // @todo why does this happen
    }

    bgfx::renderFrame(); // single threaded mode

    // Platform Data
    bgfx::PlatformData pd{};
    pd.nwh = settings.mNativeWindow;
    pd.ndt = settings.mNativeDisplay;

    // Init bgfx
    bgfx::Init bgfx_init;
    bgfx_init.type = bgfx::RendererType::Count; // auto choose renderer
    bgfx_init.resolution.width = settings.mResolutionWidth;
    bgfx_init.resolution.height = settings.mResolutionHeight;
    bgfx_init.resolution.reset = settings.mVSync ? BGFX_RESET_VSYNC : 0;
    bgfx_init.platformData = pd;

    assert(bgfx::init(bgfx_init));

}

void RenderContext::cleanup()
{
    bgfx::shutdown();
}

void RenderContext::reset(int pass, const int width, const int height)
{
    bgfx::reset(width, height);
    bgfx::setViewRect(0, 0, 0, bgfx::BackbufferRatio::Equal);
}

}	// namespace mrender