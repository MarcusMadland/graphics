#include "mrender/gfx/render_context.hpp"
#include "mrender/gfx/camera.hpp"
#include "mrender/gfx/geometry.hpp"
#include "mrender/gfx/texture.hpp"
#include "mrender/gfx/framebuffer.hpp"
#include "mrender/gfx/render_state.hpp"
#include "mrender/utils/file_ops.hpp"

#include "mrender/renderers/my-renderer/my_renderer.hpp"
#include "mrender/renderers/my-renderer2/my_renderer2.hpp"

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bgfx/platform.h>
#include <bx/math.h>
#include <string>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <bx/pixelformat.h>

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
    bgfx::Init bgfx_init;
    bgfx_init.type = bgfx::RendererType::Count; // Auto? read about it @todo Think it uses the defines in build script
    bgfx_init.vendorId = BGFX_PCI_ID_NONE; // Auto
    bgfx_init.resolution.width = mSettings.mResolutionWidth;
    bgfx_init.resolution.height = mSettings.mResolutionHeight;
    bgfx_init.resolution.reset = mResetFlags;
    bgfx_init.platformData.nwh = mSettings.mNativeWindow;
    bgfx_init.platformData.ndt = mSettings.mNativeDisplay;
    printf("Backend init result: %s\n", bgfx::init(bgfx_init) ? "Success" : "Failed");

    // @todo Add so only show in debug mode
    bgfx::setDebug(BGFX_DEBUG_TEXT);
    // bgfx::setDebug(BGFX_DEBUG_STATS);

    // Setup renderer and render sub systems
    setupRenderSystems();

    // Setup empty texture
    constexpr uint32_t width = 4;
    constexpr uint32_t height = 4;
    const bgfx::Memory* textureData = bgfx::alloc(width * height * 4);
    std::memset(textureData->data, 0, textureData->size);
    mEmptyTexture = createTexture((uint8_t*)textureData, TextureFormat::RGBA8, BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT, width, height, 4);
}

void RenderContextImplementation::cleanup()
{
    bgfx::shutdown();
}

void RenderContextImplementation::setClearColor(uint32_t rgba)
{
    mClearColor = rgba;
}

void RenderContextImplementation::writeToBuffers(std::shared_ptr<Framebuffer> framebuffer)
{
    bgfx::setViewFrameBuffer(mRenderState->mId, std::static_pointer_cast<FramebufferImplementation>(framebuffer)->mHandle);
}

void RenderContextImplementation::setRenderState(std::shared_ptr<RenderState> renderState)
{
    mRenderState = std::static_pointer_cast<RenderStateImplementation>(renderState);
}

void RenderContextImplementation::clear(uint16_t flags, uint16_t width, uint16_t height)
{
    if (width + height > 0)
    {
        bgfx::setViewRect(mRenderState->mId, 0, 0, width, height);
    }
    else
    {
        bgfx::setViewRect(mRenderState->mId, 0, 0, bgfx::BackbufferRatio::Equal);
    }

    bgfx::setViewClear(mRenderState->mId, flags, mClearColor, 1.0f, 0);
}

void RenderContextImplementation::setParameter(const std::string& shader, const std::string& uniform, std::shared_ptr<void> data, uint8_t unit)
{
    auto shaderImpl = std::static_pointer_cast<ShaderImplementation>(getShaders().at(shader));
    auto textureImpl = std::static_pointer_cast<TextureImplementation>(data);
    if (shaderImpl->mUniformHandles.count(uniform) > 0 && bgfx::isValid(shaderImpl->mUniformHandles.at(uniform).first) && textureImpl && bgfx::isValid(textureImpl->mHandle))
    {
        bgfx::setTexture(unit, shaderImpl->mUniformHandles.at(uniform).first, textureImpl->mHandle);
    }
    else if (shaderImpl->mUniformHandles.count(uniform) > 0 && bgfx::isValid(shaderImpl->mUniformHandles.at(uniform).first))
    {
        bgfx::setTexture(unit, shaderImpl->mUniformHandles.at(uniform).first, BGFX_INVALID_HANDLE);
    }
}

void RenderContextImplementation::setParameter(const std::string& shader, const std::string& uniform, std::shared_ptr<void> data)
{
    auto shaderImpl = std::static_pointer_cast<ShaderImplementation>(getShaders().at(shader));
    if (shaderImpl->mUniformHandles.count(uniform) > 0 && bgfx::isValid(shaderImpl->mUniformHandles.at(uniform).first) && data)
    {
        bgfx::setUniform(shaderImpl->mUniformHandles.at(uniform).first, data.get());
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
    bgfx::setState(mRenderState->mFlags);

    if (camera)
    {
        CameraImplementation* cameraImpl = reinterpret_cast<CameraImplementation*>(camera.get());
        bgfx::setViewTransform(mRenderState->mId, cameraImpl->getViewMatrix(), cameraImpl->getProjMatrix());
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
        bgfx::submit(mRenderState->mId, shaderImpl->mHandle, BGFX_DISCARD_STATE);
    }
}

void RenderContextImplementation::submit(const std::shared_ptr<Renderable>& renderable, const std::shared_ptr<Camera>& camera)
{
    // Set transform
    bgfx::setTransform(renderable->getTransform());

    // Set unifroms to the material data @todo texture unit should be in material data IMPORTANT
    auto shaderName = renderable->getMaterial()->getShaderName();
    auto shaderImpl = std::static_pointer_cast<ShaderImplementation>(getShaders().at(shaderName));
    for (auto iter = shaderImpl->mUniformHandles.begin(); iter != shaderImpl->mUniformHandles.end(); ++iter) 
    {
        auto key = iter->first;
        auto value = iter->second;
        auto shaderName = renderable->getMaterial()->getShaderName();

        if (renderable->getMaterial()->getUniformData().count(key) <= 0)
        {
            setParameter(shaderName, key, mEmptyTexture, value.second);
            //bgfx::setTexture(value.second, value.first, mEmptyTextureHandle);
        }
        else
        {
            const auto& uniformData = renderable->getMaterial()->getUniformData().at(key);
            if (uniformData.mType == UniformType::Sampler)
            {
                setParameter(shaderName, key, uniformData.mValue, value.second);
            }
            else
            {
                setParameter(shaderName, key, uniformData.mValue);
            }
        }
    }

    // Submit geometry for rendering
    submit(renderable->getGeometry(), renderable->getMaterial()->getShaderName(), camera);
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

        // Reset backbuffer size
        bgfx::reset(mSettings.mResolutionWidth, mSettings.mResolutionHeight, mResetFlags);
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

void RenderContextImplementation::setRenderStateCount(uint32_t stateCount)
{
    mRenderStateCount = stateCount;
}

void RenderContextImplementation::render(const std::shared_ptr<Camera>& camera)
{
    mCamera = camera;

    bgfx::touch(0); // @todo
    bgfx::dbgTextClear();

    // Render all subsystems of the renderer
    for (auto& renderSystem : mRenderSystems)
    {
        renderSystem->render(*this);
    }
}

void RenderContextImplementation::swapBuffers()
{
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
    setRenderStateCount(0);

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
        for (auto& buffer : renderSystem->getBuffers(*this))
        {
            mBuffers[buffer.first] = std::move(buffer.second);
        }

        renderSystem->init(*this);
    }

    return true;
}

void RenderContextImplementation::setupResetFlags()
{
    mResetFlags = mSettings.mVSync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE;
}

}	// namespace mrender