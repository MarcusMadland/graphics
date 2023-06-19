#include "mrender/handler/render_context.hpp"
#include "mrender/handler/camera.hpp"
#include "mrender/handler/geometry.hpp"
#include "mrender/handler/framebuffer.hpp"
#include "mrender/handler/texture.hpp"
#include "mrender/core/file_ops.hpp"

#include "mrender/renderers/my-renderer/my_renderer.hpp"
#include "mrender/renderers/my-renderer2/my_renderer2.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>
#include <string>
#include <iostream>
#include <cassert>
#include <cstdlib>

namespace mrender {

void RenderContextImplementation::initialize(const RenderSettings& settings)
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

void RenderContextImplementation::cleanup()
{
    bgfx::shutdown();
}

void RenderContextImplementation::setClearColor(uint32_t rgba)
{
    mClearColor = rgba;
}

void RenderContextImplementation::writeToBuffer(const std::string_view& buffer, bool writeToBackBuffer)
{
    std::shared_ptr<FrameBufferImplementation> frameBufferImpl = std::static_pointer_cast<FrameBufferImplementation>(mBuffers[buffer.data()]);
    mCurrentRenderPass = frameBufferImpl->mId;
    
    if (!writeToBackBuffer)
    {
        if (frameBufferImpl->width + frameBufferImpl->height != 0)
        {
            bgfx::setViewRect(mCurrentRenderPass, 0, 0, frameBufferImpl->width, frameBufferImpl->height);
        }
        else
        {
            bgfx::setViewRect(mCurrentRenderPass, 0,0, bgfx::BackbufferRatio::Equal);
        }

        bgfx::setViewFrameBuffer(mCurrentRenderPass, frameBufferImpl->mHandle);
    }
    else
    {
        bgfx::setViewFrameBuffer(mCurrentRenderPass, BGFX_INVALID_HANDLE);
    }

}

void RenderContextImplementation::clear()
{
    bgfx::setViewClear(mCurrentRenderPass, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, mClearColor, 1.0f, 0);
}

void RenderContextImplementation::setParameter(const std::string_view& shader, const std::string_view& uniform, const std::shared_ptr<Texture>& texture)
{
    std::shared_ptr<ShaderImplementation> shaderImpl = std::static_pointer_cast<ShaderImplementation>(mShaders.at(shader.data()));
    if (shaderImpl.get() == nullptr || shaderImpl == nullptr) std::cout << "Invalid shader" << std::endl;
    
    std::shared_ptr<TextureImplementation> textureImpl = std::static_pointer_cast<TextureImplementation>(texture);
    if (textureImpl.get() == nullptr || textureImpl == nullptr) std::cout << "Invalid texture" << std::endl;

    // Retrieve the shader uniforms
    if (shaderImpl->mUniformHandles.count(uniform.data()) != 0)
    {
        bgfx::UniformHandle uniformHandle = shaderImpl->mUniformHandles[uniform.data()].mHandle;
        const uint8_t unit = shaderImpl->mUniformHandles[uniform.data()].unit;

        if (bgfx::isValid(uniformHandle) && bgfx::isValid(textureImpl->mHandle))
        {
           // printf("binding unit %s %u \n", uniform.data(), shaderImpl->mUniformHandles[uniform.data()].unit);
           bgfx::setTexture(unit, uniformHandle, textureImpl->mHandle);
        }
        else
        {
            //std::cout << "invalid uniform" << std::endl;
        }
    }
    else
    {
        //printf("Could not find %s uniform in uniform map of size %u \n", uniform.data(), shaderImpl->mUniformHandles.size());
    }
    
}

void RenderContextImplementation::submitDebugTextOnScreen(uint16_t x, uint16_t y, std::string_view text, ...)
{
    constexpr int maxBufferSize = 256;
    char buffer[maxBufferSize];

    va_list args;
    va_start(args, text);

    //std::vsprintf(buffer, text.data(), args);
    vsprintf_s(buffer, text.data(), args);

    va_end(args);

    const bgfx::Stats* stats = bgfx::getStats();
    bgfx::dbgTextPrintf(stats->textWidth - x, y, 0x0f, buffer);
}

void RenderContextImplementation::submitDebugTextOnScreen(uint16_t x, uint16_t y, Color color, std::string_view text, ...)
{
    constexpr int maxBufferSize = 256;
    char buffer[maxBufferSize];

    va_list args;
    va_start(args, text);

    //std::vsprintf(buffer, text.data(), args);
    vsprintf_s(buffer, text.data(), args);

    va_end(args);

    const bgfx::Stats* stats = bgfx::getStats();
    bgfx::dbgTextPrintf(stats->textWidth - x, y, colorToAnsi(color), buffer);
}

void RenderContextImplementation::submitDebugTextOnScreen(uint16_t x, uint16_t y, Color color, bool right, bool top, std::string_view text, ...)
{
    constexpr int maxBufferSize = 256;
    char buffer[maxBufferSize];

    va_list args;
    va_start(args, text);

    //std::vsprintf(buffer, text.data(), args);
    vsprintf_s(buffer, text.data(), args);

    va_end(args);

    const bgfx::Stats* stats = bgfx::getStats();
    bgfx::dbgTextPrintf(right ? stats->textWidth - x : x, top ? y : stats->textHeight - y, colorToAnsi(color), buffer);
}

void RenderContextImplementation::submit(const std::shared_ptr<Geometry>& geometry, const std::string_view& shaderName, const std::shared_ptr<Camera>& camera)
{
    if (camera)
    {
        CameraImplementation* cameraImpl = reinterpret_cast<CameraImplementation*>(camera.get());
        bgfx::setViewTransform(mCurrentRenderPass, cameraImpl->getViewMatrix(), cameraImpl->getProjMatrix());
    }

    GeometryImplementation* geometryImpl = reinterpret_cast<GeometryImplementation*>(geometry.get());
    if (geometryImpl)
    {
        bgfx::setVertexBuffer(0, geometryImpl->mVertexBufferHandle);
        bgfx::setIndexBuffer(geometryImpl->mIndexBufferHandle);
    }
    
    ShaderImplementation* shaderImpl = reinterpret_cast<ShaderImplementation*>(mShaders[shaderName.data()].get());
    if (shaderImpl)
    {
        bgfx::submit(mCurrentRenderPass, shaderImpl->mHandle);
    }
}

void RenderContextImplementation::submit(const std::shared_ptr<Renderable>& renderable, const std::shared_ptr<Camera>& camera)
{
    bgfx::setTransform(renderable->getTransform());
    submit(renderable->getGeometry(), renderable->getShader(), camera);
}

void RenderContextImplementation::submit(const std::vector<std::shared_ptr<Renderable>>& renderables, const std::shared_ptr<Camera>& camera)
{
    for (auto& renderable : renderables)
    {
        submit(renderable, camera);
    }
}

void RenderContextImplementation::loadShader(char const* fileName, char const* filePath)
{
    std::shared_ptr<ShaderImplementation> shader = std::make_shared<ShaderImplementation>();
    shader->loadProgram(fileName, filePath);
    mShaders[fileName] = std::move(shader);
}

void RenderContextImplementation::reloadShaders()
{
    for (auto& shader : mShaders)
    {
        shader.second->reloadProgram();
    }
}

void RenderContextImplementation::setSettings(const RenderSettings& settings)
{
    const bool resetViewAndFlags = (mSettings.mVSync != settings.mVSync || mSettings.mResolutionWidth != settings.mResolutionWidth || mSettings.mResolutionHeight != settings.mResolutionHeight);
    const bool rebuildRenderer = (mSettings.mRendererName != settings.mRendererName || mRenderer == nullptr) || resetViewAndFlags;

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
       // bgfx::Clear(flags) @todo
    }
}

void RenderContextImplementation::addRenderable(std::shared_ptr<Renderable> renderable)
{
    mRenderables.push_back(std::move(renderable));
}

void RenderContextImplementation::setRenderables(std::vector<std::shared_ptr<Renderable>> renderables)
{
    mRenderables = std::move(renderables);
}

void RenderContextImplementation::addBuffer(const std::string_view& name, std::shared_ptr<FrameBuffer> buffer)
{
    mBuffers[name.data()] = std::move(buffer);
}

void RenderContextImplementation::setPassCount(uint32_t passCount)
{
    mRenderPassCount = passCount;
}

void RenderContextImplementation::render(const std::shared_ptr<Camera>& camera)
{
    mCamera = camera;

    bgfx::touch(mRenderPassCount - 1);
    bgfx::dbgTextClear();

    // Render all subsystems of the renderer
    for (auto& renderSystem : mRenderSystems)
    {
        renderSystem->render(*this);
    }
}

void RenderContextImplementation::frame()
{
    bgfx::setDebug(BGFX_DEBUG_TEXT);

    // Backend frame call
    bgfx::frame();
}

uint8_t RenderContextImplementation::colorToAnsi(const Color& color)
{
    switch (color)
    {
    case mrender::Color::White:
        return UINT8_C(15);
        break;
    case mrender::Color::Red:
        return UINT8_C(4);
        break;
    case mrender::Color::Blue:
        return UINT8_C(1);
        break;
    case mrender::Color::Green:
        return UINT8_C(10);
        break;
    default:
        return UINT8_C(15);
        break;
    }

    return UINT8_C(15);
}

bool RenderContextImplementation::setupRenderSystems()
{
    // Clear renderPasses
    mRenderPassCount = 0;

    // Create the renderer from name in settings
    mRenderer = Renderer::make(mSettings.mRendererName);
    if (mRenderer == nullptr)
    {
        return false;
    }

    // Clear and set up all sub systems of created renderer
    mRenderSystems.clear();
    mRenderSystems = std::move(mRenderer->setupRenderSystems(*this));

    // Clear buffers
    mBuffers.clear();

    // Initialize all render sub systems
    for (auto& renderSystem : mRenderSystems)
    {
        renderSystem->init(*this);
        for (auto& buffer : renderSystem->getBuffers(*this))
        {
            bgfx::setViewName(std::static_pointer_cast<FrameBufferImplementation>(buffer.second)->mId, buffer.first.data());
            mBuffers[buffer.first] = std::move(buffer.second);
        }
    }

    return true;
}

void RenderContextImplementation::setupResetFlags()
{
    mResetFlags = mSettings.mVSync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE;
}

}	// namespace mrender