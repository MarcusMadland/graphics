#include "mrender/gfx/render_context.hpp"
#include "mrender/gfx/camera.hpp"
#include "mrender/gfx/geometry.hpp"
#include "mrender/gfx/texture.hpp"
#include "mrender/gfx/framebuffer.hpp"
#include "mrender/gfx/material.hpp"
#include "mrender/gfx/renderable.hpp"
#include "mrender/gfx/render_state.hpp"
#include "mrender/utils/file_ops.hpp"

#include "mrender/renderers/deferred/deferred.hpp"
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

GfxContextImplementation::GfxContextImplementation(const RenderSettings& settings)
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
    bgfx::init(bgfx_init);

    bgfx::setDebug(mSettings.mRenderDebugText ? BGFX_DEBUG_TEXT : BGFX_DEBUG_NONE);
    //bgfx::setDebug(BGFX_DEBUG_STATS);

    // Setup renderer and render sub systems
    setupRenderSystems();

    // Setup empty texture
    constexpr uint32_t width = 128;
    constexpr uint32_t height = 128;
    const bgfx::Memory* textureData = bgfx::alloc(width * height * 4);
    std::memset(textureData->data, 0, textureData->size);
    mEmptyTexture = createTexture((uint8_t*)textureData, TextureFormat::RGBA8, BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT, width, height, 4);

}

GfxContextImplementation::~GfxContextImplementation()
{
    //bgfx::shutdown();
}

CameraHandle GfxContextImplementation::createCamera(const CameraSettings& settings)
{
    CameraHandle handle = { static_cast<uint16_t>(mCameras.size()) };
    mCameras[handle.idx] = std::move(std::make_shared<CameraImplementation>(settings));
    return handle;
}

FramebufferHandle GfxContextImplementation::createFramebuffer(BufferList buffers)
{
    FramebufferHandle handle = { static_cast<uint16_t>(mFramebuffers.size()) };
    mFramebuffers[handle.idx] = std::move(std::make_shared<FramebufferImplementation>(this, buffers));
    return handle;
}

RenderStateHandle GfxContextImplementation::createRenderState(std::string_view name, uint64_t flags)
{
    RenderStateHandle handle = { static_cast<uint16_t>(mRenderStates.size()) };
    mRenderStates[handle.idx] = std::move(std::make_shared<RenderStateImplementation>(this, name, flags));
    return handle;
}

MaterialHandle GfxContextImplementation::createMaterial(ShaderHandle shader)
{
    MaterialHandle handle = { static_cast<uint16_t>(mMaterials.size()) };
    mMaterials[handle.idx] = std::move(std::make_shared<MaterialImplementation>(this, shader));
    return handle;
}

TextureHandle GfxContextImplementation::createTexture(TextureFormat format, uint64_t textureFlags, uint16_t width, uint16_t height)
{
    TextureHandle handle = { static_cast<uint16_t>(mTextures.size()) };
    mTextures[handle.idx] = std::move(std::make_shared<TextureImplementation>( format, textureFlags, width, height));
    return handle;
}

TextureHandle GfxContextImplementation::createTexture(const uint8_t* data, TextureFormat format, uint64_t textureFlags, uint16_t width, uint16_t height, uint16_t channels)
{
    TextureHandle handle = { static_cast<uint16_t>(mTextures.size()) };
    mTextures[handle.idx] = std::move(std::make_shared<TextureImplementation>(data, format, textureFlags, width, height, channels));
    return handle;
}

ShaderHandle GfxContextImplementation::createShader(const std::string& fileName, const std::string& filePath)
{
    ShaderHandle handle = { static_cast<uint16_t>(mShaders.size()) };
    std::shared_ptr<ShaderImplementation> shader = std::make_shared<ShaderImplementation>();
    shader->loadProgram(fileName, filePath);
    mShaders[handle.idx] = std::move(shader);
    return handle;
}

GeometryHandle GfxContextImplementation::createGeometry(const BufferLayout& layout, void* vertexData, uint32_t vertexSize, const std::vector<uint16_t>& indices)
{
    GeometryHandle handle = { static_cast<uint16_t>(mGeometries.size()) };
    mGeometries[handle.idx] = std::move(std::make_shared<GeometryImplementation>(layout, vertexData, vertexSize, indices));
    return handle;
}

RenderableHandle GfxContextImplementation::createRenderable(GeometryHandle geometry, MaterialHandle material)
{
    RenderableHandle handle = { static_cast<uint16_t>(mRenderables.size()) };
    mRenderables[handle.idx] = std::move(std::make_shared<RenderableImplementation>(geometry, material));
    return handle;
}

void GfxContextImplementation::destroy(CameraHandle handle)
{
    mCameras.erase(handle.idx);
}

void GfxContextImplementation::destroy(FramebufferHandle handle)
{
    mFramebuffers.erase(handle.idx);
}

void GfxContextImplementation::destroy(RenderStateHandle handle)
{
    mRenderStates.erase(handle.idx);
}

void GfxContextImplementation::destroy(MaterialHandle handle)
{
    mMaterials.erase(handle.idx);
}

void GfxContextImplementation::destroy(TextureHandle handle)
{
    mTextures.erase(handle.idx);
}

void GfxContextImplementation::destroy(ShaderHandle handle)
{
    mShaders.erase(handle.idx);
}

void GfxContextImplementation::destroy(GeometryHandle handle)
{
    mGeometries.erase(handle.idx);
}

void GfxContextImplementation::destroy(RenderableHandle handle)
{
    mRenderables.erase(handle.idx);
}

void GfxContextImplementation::render(CameraHandle camera)
{
    mCurrentCamera = camera;

    // Update stats
    updateStats();

    // @todo
    bgfx::touch(0); 

    // Clear debug text buffer
    bgfx::dbgTextClear();

    // Render all subsystems of the renderer
    for (auto& renderSystem : mRenderSystems)
    {
        renderSystem->render(this);
    }
}

void GfxContextImplementation::swapBuffers()
{
    // Backend swap buffer call
    bgfx::frame();
}

void GfxContextImplementation::clear(uint16_t flags, uint16_t width, uint16_t height)
{
    auto renderStateImpl = STATIC_IMPL_CAST(RenderState, mRenderStates.at(mCurrentRenderState.idx));

    if (width + height > 0)
    {
        bgfx::setViewRect(renderStateImpl->mId, 0, 0, width, height);
    }
    else
    {
        bgfx::setViewRect(renderStateImpl->mId, 0, 0, bgfx::BackbufferRatio::Equal);
    }

    bgfx::setViewClear(renderStateImpl->mId, flags, mClearColor, 1.0f, 0);
}

void GfxContextImplementation::setClearColor(uint32_t rgba)
{
    mClearColor = rgba;
}

void GfxContextImplementation::reloadShaders()
{
    for (auto& shader : mShaders)
    {
        auto shaderImpl = STATIC_IMPL_CAST(Shader, shader.second);
        shaderImpl->reloadProgram();
    }
}

void GfxContextImplementation::setActiveRenderState(RenderStateHandle renderState)
{
    mCurrentRenderState = renderState;
}

void GfxContextImplementation::setActiveFramebuffer(FramebufferHandle framebuffer)
{
    auto renderStateImpl = STATIC_IMPL_CAST(RenderState, mRenderStates.at(mCurrentRenderState.idx));
    auto framebufferImpl = STATIC_IMPL_CAST(Framebuffer, mFramebuffers.at(framebuffer.idx));

    bgfx::setViewFrameBuffer(renderStateImpl->mId, framebufferImpl->mHandle);
}

void GfxContextImplementation::setActiveRenderable(RenderableHandle renderable)
{
    mCurrentRenderables.push_back(renderable);
}

void GfxContextImplementation::setActiveRenderables(std::vector<RenderableHandle> renderables)
{
    for (auto& renderable : renderables)
    {
        mCurrentRenderables.push_back(renderable);
    }
}

void GfxContextImplementation::submitDebugText(uint16_t x, uint16_t y, std::string_view text, ...)
{
    constexpr int maxBufferSize = 256;
    char buffer[maxBufferSize];

    va_list args;
    va_start(args, text);
    vsprintf_s(buffer, text.data(), args);
    va_end(args);

    const bgfx::Stats* stats = bgfx::getStats();
    bgfx::dbgTextPrintf(stats->textWidth - x, y, 0x0f, buffer);
}

void GfxContextImplementation::submitDebugText(uint16_t x, uint16_t y, Color color, std::string_view text, ...)
{
    constexpr int maxBufferSize = 256;
    char buffer[maxBufferSize];

    va_list args;
    va_start(args, text);

    vsprintf_s(buffer, text.data(), args);

    va_end(args);

    const bgfx::Stats* stats = bgfx::getStats();
    bgfx::dbgTextPrintf(stats->textWidth - x, y, colorToAnsi(color), buffer);
}

void GfxContextImplementation::submitDebugText(uint16_t x, uint16_t y, Color color, bool right, bool top, std::string_view text, ...)
{
    constexpr int maxBufferSize = 256;
    char buffer[maxBufferSize];

    va_list args;
    va_start(args, text);

    vsprintf_s(buffer, text.data(), args);

    va_end(args);

    const bgfx::Stats* stats = bgfx::getStats();
    bgfx::dbgTextPrintf(right ? stats->textWidth - x : x, top ? y : stats->textHeight - y, colorToAnsi(color), buffer);
}

void GfxContextImplementation::submit(GeometryHandle geometry, ShaderHandle shader, CameraHandle camera)
{
    auto renderStateImpl = STATIC_IMPL_CAST(RenderState, mRenderStates.at(mCurrentRenderState.idx));
    auto geometryImpl = STATIC_IMPL_CAST(Geometry, mGeometries.at(geometry.idx));
    auto shaderImpl = STATIC_IMPL_CAST(Shader, mShaders.at(shader.idx));
    auto cameraImpl = isValid(camera) ? STATIC_IMPL_CAST(Camera, mCameras.at(camera.idx)) : nullptr;

    if (!bgfx::isValid(shaderImpl->mHandle))
    {
        return;
    }

    // Set render states for this draw call
    bgfx::setState(renderStateImpl->mFlags);

    // If camera is valid we set projection and view matrix uniforms
    if (cameraImpl)
    {
        bgfx::setViewTransform(renderStateImpl->mId, cameraImpl->getViewMatrix(), cameraImpl->getProjMatrix());
    }

    // Set geometry data to render
    bgfx::setVertexBuffer(0, geometryImpl->mVertexBufferHandle);
    bgfx::setIndexBuffer(geometryImpl->mIndexBufferHandle);

    // Submit drawcall
    bgfx::submit(renderStateImpl->mId, shaderImpl->mHandle, BGFX_DISCARD_STATE);
}

void GfxContextImplementation::submit(RenderableHandle renderable, CameraHandle camera)
{
    auto renderableImpl = STATIC_IMPL_CAST(Renderable, mRenderables.at(renderable.idx));
    auto cameraImpl = STATIC_IMPL_CAST(Camera, mCameras.at(camera.idx));
    auto materialImpl = STATIC_IMPL_CAST(Material, mMaterials.at(renderableImpl->getMaterial().idx));
    auto shaderImpl = STATIC_IMPL_CAST(Shader, mShaders.at(materialImpl->getShader().idx));

    // Set transform
    bgfx::setTransform(renderableImpl->getTransform());

    // Go over all uniforms required by the shader and set it to correct value from material data
    for (auto uniformHandle : shaderImpl->mUniformHandles)
    {
        std::string key = uniformHandle.first;
        std::pair<bgfx::UniformHandle, uint8_t> value = uniformHandle.second;

        if (!bgfx::isValid(value.first))
        {
            return;
        }

        bgfx::UniformInfo uniformInfo;
        bgfx::getUniformInfo(value.first, uniformInfo);

        // If material contains data for the uniform we set the value depending on if the uniform is a sampler or other
        if (materialImpl->getTextureDataList().count(key) > 0 || materialImpl->getUniformDataList().count(key) > 0)
        {
            if (uniformInfo.type == bgfx::UniformType::Sampler)
            {
                TextureHandle texture = materialImpl->getTextureDataList().at(key);
                setTexture(materialImpl->getShader(), key, texture, value.second);
            }
            else
            {
                const UniformData& uniformData = materialImpl->getUniformDataList().at(key);
                setUniform(materialImpl->getShader(), key, uniformData.mValue);
            }
        }

        // If material contains no data for the uniform we set to it null or empty texture depending on if the uniform is a sampler or other
        else
        {
            switch (uniformInfo.type)
            {
                case bgfx::UniformType::Sampler:
                {
                    setTexture(materialImpl->getShader(), key, mEmptyTexture, value.second);
                    break;
                }

                case bgfx::UniformType::Vec4:
                {
                    static float data[4] = { }; 
                    bgfx::setUniform(shaderImpl->mUniformHandles.at(key).first, &data);
                    break;
                }
                
                case bgfx::UniformType::Mat3:
                {
                    static float data[9] = { };
                    bgfx::setUniform(shaderImpl->mUniformHandles.at(key).first, &data);
                    break;
                }

                case bgfx::UniformType::Mat4:
                {
                    static float data[16] = { };
                    bgfx::setUniform(shaderImpl->mUniformHandles.at(key).first, &data);
                    break;
                }
            }
        }
    }

    // Submit geometry for rendering
    submit(renderableImpl->getGeometry(), materialImpl->getShader(), camera);
}

void GfxContextImplementation::submit(RenderableList renderables, CameraHandle camera)
{
    // Submit renderables for rendering
    for (const auto& renderable : renderables)
    {
        submit(renderable, camera);
    }
}

void GfxContextImplementation::submit(RenderableHandle renderable, ShaderHandle shader, CameraHandle camera)
{
    auto renderableImpl = STATIC_IMPL_CAST(Renderable, mRenderables.at(renderable.idx));
 
    // Set transform
    bgfx::setTransform(renderableImpl->getTransform());
    
    // Submit geometry for rendering
    submit(renderableImpl->getGeometry(), shader, camera);
}

void GfxContextImplementation::submit(RenderableList renderables, ShaderHandle shader, CameraHandle camera)
{
    // Submit renderables for rendering
    for (const auto& renderable : renderables)
    {
        submit(renderable, shader, camera);
    }
}

void GfxContextImplementation::setTexture(ShaderHandle shader, const std::string& uniform, TextureHandle texture, uint8_t unit)
{
    auto shaderImpl = STATIC_IMPL_CAST(Shader, mShaders.at(shader.idx));
    auto textureImpl = STATIC_IMPL_CAST(Texture, getTextureData(texture));

    if (shaderImpl->mUniformHandles.count(uniform) > 0 && bgfx::isValid(shaderImpl->mUniformHandles.at(uniform).first) && textureImpl && bgfx::isValid(textureImpl->mHandle))
    {
        bgfx::setTexture(unit, shaderImpl->mUniformHandles.at(uniform).first, textureImpl->mHandle);
    }
    else if (shaderImpl->mUniformHandles.count(uniform) > 0 && bgfx::isValid(shaderImpl->mUniformHandles.at(uniform).first))
    {
        bgfx::setTexture(unit, shaderImpl->mUniformHandles.at(uniform).first, BGFX_INVALID_HANDLE);
    }
}

void GfxContextImplementation::setUniform(ShaderHandle shader, const std::string& uniform, void* data)
{
    auto shaderImpl = STATIC_IMPL_CAST(Shader, mShaders.at(shader.idx));

    if (shaderImpl->mUniformHandles.count(uniform) > 0 && bgfx::isValid(shaderImpl->mUniformHandles.at(uniform).first) && data)
    {
        bgfx::setUniform(shaderImpl->mUniformHandles.at(uniform).first, data);
    }
}

CameraSettings GfxContextImplementation::getCameraSettings(CameraHandle camera)
{
    std::shared_ptr<CameraImplementation> cameraImpl = STATIC_IMPL_CAST(Camera, mCameras.at(camera.idx));
    return cameraImpl->getSettings();
}

float* GfxContextImplementation::getCameraProjection(CameraHandle camera)
{
    std::shared_ptr<CameraImplementation> cameraImpl = STATIC_IMPL_CAST(Camera, mCameras.at(camera.idx));
    return cameraImpl->getProjMatrix();
}

void GfxContextImplementation::setCameraSettings(CameraHandle camera, const CameraSettings& settings)
{
    std::shared_ptr<CameraImplementation> cameraImpl = STATIC_IMPL_CAST(Camera, mCameras.at(camera.idx));
    cameraImpl->setSettings(settings);
}

void GfxContextImplementation::setMaterialUniformData(MaterialHandle material, const std::string& name, UniformData::UniformType type, void* data)
{
    std::shared_ptr<MaterialImplementation> materialImpl = STATIC_IMPL_CAST(Material, mMaterials.at(material.idx));
    materialImpl->setUniformData(name, type, data);
}

void GfxContextImplementation::setMaterialTextureData(MaterialHandle material, const std::string& name, TextureHandle texture)
{
    std::shared_ptr<MaterialImplementation> materialImpl = STATIC_IMPL_CAST(Material, mMaterials.at(material.idx));
    materialImpl->setTextureData(name, texture);
}

const UniformDataList& GfxContextImplementation::getMaterialUniformData(MaterialHandle material)
{
    std::shared_ptr<MaterialImplementation> materialImpl = STATIC_IMPL_CAST(Material, mMaterials.at(material.idx));
    return materialImpl->getUniformDataList();
}

const TextureDataList& GfxContextImplementation::getMaterialTextureData(MaterialHandle material)
{
    std::shared_ptr<MaterialImplementation> materialImpl = STATIC_IMPL_CAST(Material, mMaterials.at(material.idx));
    return materialImpl->getTextureDataList();
}

const ShaderHandle GfxContextImplementation::getMaterialShader(MaterialHandle material)
{
    std::shared_ptr<MaterialImplementation> materialImpl = STATIC_IMPL_CAST(Material, mMaterials.at(material.idx));
    return materialImpl->getShader();
}

TextureRef GfxContextImplementation::getTextureData(TextureHandle texture)
{
    return mTextures.at(texture.idx);
}

TextureFormat GfxContextImplementation::getTextureFormat(TextureHandle texture)
{
    auto textureImpl = STATIC_IMPL_CAST(Texture, mTextures.at(texture.idx));
    return textureImpl->getFormat();
}

void GfxContextImplementation::setRenderableMaterial(RenderableHandle renderable, MaterialHandle material)
{
    auto renderableImpl = STATIC_IMPL_CAST(Renderable, mRenderables.at(renderable.idx));
    renderableImpl->setMaterial(material);
}

void GfxContextImplementation::setRenderableTransform(RenderableHandle renderable, float* matrix)
{
    auto renderableImpl = STATIC_IMPL_CAST(Renderable, mRenderables.at(renderable.idx));
    renderableImpl->setTransform(matrix);
}

float* GfxContextImplementation::getRenderableTransform(RenderableHandle renderable)
{
    auto renderableImpl = STATIC_IMPL_CAST(Renderable, mRenderables.at(renderable.idx));
    return renderableImpl->getTransform();
}

void GfxContextImplementation::setSettings(const RenderSettings& settings)
{
    const bool rebuildRenderer = (mSettings.mRendererName != settings.mRendererName || mRenderer == nullptr);
    const bool resetViewAndFlags = (mSettings.mVSync != settings.mVSync || mSettings.mResolutionWidth != settings.mResolutionWidth || mSettings.mResolutionHeight != settings.mResolutionHeight) || rebuildRenderer;

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

void GfxContextImplementation::setRenderStateCount(uint32_t stateCount)
{
    mRenderStateCount = stateCount;
}

void GfxContextImplementation::updateStats()
{
    const bgfx::Stats* stats = bgfx::getStats();
    const double toCpuMs = 1000.0 / double(stats->cpuTimerFreq);
    const double toGpuMs = 1000.0 / double(stats->gpuTimerFreq);

    mStats.mCpuTime = float((stats->cpuTimeEnd - stats->cpuTimeBegin) * toCpuMs);
    mStats.mGpuTime = float((stats->gpuTimeEnd - stats->gpuTimeBegin) * toGpuMs);

    mStats.mNumDrawCalls = stats->numDraw;
    mStats.mTextureMemoryUsed = stats->textureMemoryUsed / (1024 * 1024);

    mStats.mNumCameras = static_cast<uint32_t>(mCameras.size());
    mStats.mNumFramebuffers = static_cast<uint32_t>(mFramebuffers.size());
    mStats.mNumRenderStates = static_cast<uint32_t>(mRenderStates.size());
    mStats.mNumMaterials = static_cast<uint32_t>(mMaterials.size());
    mStats.mNumTextures = static_cast<uint32_t>(mTextures.size());
    mStats.mNumShaders = static_cast<uint32_t>(mShaders.size());
    mStats.mNumGeometries = static_cast<uint32_t>(mGeometries.size());
    mStats.mNumRenderables = static_cast<uint32_t>(mRenderables.size());
}

uint8_t GfxContextImplementation::colorToAnsi(const Color& color)
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

        // @todo add support for more colors
    }

    return UINT8_C(15);
}

bool GfxContextImplementation::setupRenderSystems()
{
    // Clear render states
    setRenderStateCount(0);
    mRenderStates.clear();
    mFramebuffers.clear();
    // @todo what to clear? since data could be created anywhere?

    // Create the renderer from name in settings
    mRenderer = Renderer::make(mSettings.mRendererName);
    if (mRenderer == nullptr)
    {
        return false;
    }

    // Clear and set up all sub systems of created renderer
    mRenderSystems.clear();
    mRenderSystems = std::move(mRenderer->setupRenderSystems(this));

    // Clear buffers @todo is this ok?
    /*
    for (auto sharedBuffer : mSharedBuffers)
    {
        destroy(sharedBuffer.second);
    }
    mSharedBuffers.clear(); */

    // Initialize all render sub systems
    for (auto& renderSystem : mRenderSystems)
    {
        for (auto& buffer : renderSystem->getBuffers(this))
        {
            mSharedBuffers[buffer.first] = buffer.second;
        }

        renderSystem->init(this);

        for (auto& uniformData : renderSystem->getUniformData(this))
        {
            mSharedUniformData[uniformData.first] = uniformData.second;
        }
    }

    return true;
}

void GfxContextImplementation::setupResetFlags()
{
    mResetFlags = mSettings.mVSync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE;
}

}	// namespace mrender