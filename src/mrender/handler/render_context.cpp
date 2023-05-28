#include "mrender/handler/render_context.hpp"
#include "mrender/handler/shader.hpp"
#include "mrender/core/file_ops.hpp"
#include "mrender/renderers/renderer.hpp"

#include "mrender/renderers/my-renderer/my_renderer.hpp"
#include "mrender/renderers/my-renderer2/my_renderer2.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>
#include <string>
#include <iostream>
#include <cassert>

namespace mrender {

void RenderContext::initialize(const RenderSettings& settings)
{
    mSettings = settings;

    if (mSettings.mNativeWindow == nullptr)
    {
        return; // @todo why does this happen
    }

    bgfx::renderFrame(); // @todo This is single threaded mode, add setting for this?
    setupResetFlags();
     
    // Setup render backend
    bgfx::PlatformData pd{};
    pd.nwh = mSettings.mNativeWindow;
    pd.ndt = mSettings.mNativeDisplay;

    bgfx::Init bgfx_init;
    bgfx_init.type = bgfx::RendererType::Count; // auto? read about it @todo
    bgfx_init.resolution.width = mSettings.mResolutionWidth;
    bgfx_init.resolution.height = mSettings.mResolutionHeight;
    bgfx_init.resolution.reset = mResetFlags;
    bgfx_init.platformData = pd;

    assert(bgfx::init(bgfx_init));

    // Setup renderer and render sub systems
    setupRenderSystems();
}

void RenderContext::cleanup()
{
    bgfx::shutdown();
}

void RenderContext::reset(const int pass, const int width, const int height)
{
    mSettings.mResolutionWidth = width;
    mSettings.mResolutionHeight = height;
    reset(pass);
}

void RenderContext::reset(const int pass)
{
    bgfx::reset(mSettings.mResolutionWidth, mSettings.mResolutionHeight, mResetFlags);
    bgfx::setViewRect(pass, 0, 0, bgfx::BackbufferRatio::Equal);
}

void RenderContext::submitDebugTextOnScreen(uint16_t x, uint16_t y, std::string_view text, ...)
{
    constexpr int maxBufferSize = 256;
    char buffer[maxBufferSize];

    va_list args;
    va_start(args, text);

    std::vsprintf(buffer, text.data(), args);

    va_end(args);

    const bgfx::Stats* stats = bgfx::getStats();
    bgfx::dbgTextPrintf(stats->textWidth - x, y, 0x0f, buffer);
}
/*
void RenderContext::addShader(std::unique_ptr<Shader> shader)
{
    mShaders.push_back(std::move(shader));
}

void RenderContext::reloadShaders()
{
    for (auto& shader : mShaders)
    {
        shader->reloadProgram();
    }
}*/

void RenderContext::render(const RenderSettings& settings)
{
    bgfx::touch(0);
    bgfx::dbgTextClear();

    const bool rebuildRenderer = (mSettings.mRendererName != settings.mRendererName || mRenderer == nullptr);
    const bool resetViewAndFlags = (mSettings.mVSync != settings.mVSync || mSettings.mResolutionWidth != settings.mResolutionWidth || mSettings.mResolutionHeight != settings.mResolutionHeight);

    mSettings = settings;

    // Rebuild renderer and all its sub systems if renderer has been changed
    if (rebuildRenderer)
    {
        std::cout << "Rebuilding render systems..." << std::endl;
        setupRenderSystems();
    }

    // Reset viewport and all flags if any flag settings has been changed
    if (resetViewAndFlags)
    {
        std::cout << "Reset viewport and flags..." << std::endl;
        setupResetFlags();
        reset(0);
    }

    // Render all subsystems of the renderer
    for (auto& renderSystem : mRenderSystems)
    {
        renderSystem->render(*this);
    }

    // Update internal stats from backend and other sources
    const bgfx::Stats* stats = bgfx::getStats();
    mStats.mNumDrawCalls = stats->numDraw;
}

void RenderContext::frame()
{
    bgfx::setDebug(BGFX_DEBUG_TEXT);

    // Backend frame call
    bgfx::frame();
}

bool RenderContext::setupRenderSystems()
{
    // Create the renderer from name in settings
    mRenderer = Renderer::make(mSettings.mRendererName);
    if (mRenderer == nullptr)
    {
        return false;
    }

    // Clear and set up all sub systems of created renderer
    mRenderSystems.clear();
    mRenderSystems = std::move(mRenderer->setupRenderSystems(*this));

    // Initialize all render sub systems
    for (auto& renderSystem : mRenderSystems)
    {
        renderSystem->init(*this);
    }

    return true;
}

void RenderContext::setupResetFlags()
{
    mResetFlags = mSettings.mVSync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE;
}

}	// namespace mrender