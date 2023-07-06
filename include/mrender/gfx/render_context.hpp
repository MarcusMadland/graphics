#pragma once

#include "mrender/mrender.hpp"
#include "mrender/gfx/shader.hpp"
#include "mrender/gfx/render_state.hpp"

#include <string_view>
#include <vector>
#include <memory>
#include <cstdarg> 
#include <unordered_map>

namespace mrender {

class GfxContextImplementation : public GfxContext
{
	friend class CameraImplementation;
	friend class FramebufferImplementation;
	friend class RenderStateImplementation;
	friend class MaterialImplementation;
	friend class TextureImplementation;
	friend class ShaderImplementation;
	friend class GeometryImplementation;
	friend class RenderableImplementation;

public:
	GfxContextImplementation(const RenderSettings& settings);
	~GfxContextImplementation();

	virtual CameraHandle createCamera(const CameraSettings& settings) override;
	virtual FramebufferHandle createFramebuffer(BufferList buffers) override;
	virtual RenderStateHandle createRenderState(std::string_view name, uint64_t flags) override;
	virtual MaterialHandle createMaterial(ShaderHandle shader) override;
	virtual TextureHandle createTexture(TextureFormat format, uint64_t textureFlags, uint16_t width = 0, uint16_t height = 0) override;
	virtual TextureHandle createTexture(const uint8_t* data, TextureFormat format, uint64_t textureFlags, uint16_t width, uint16_t height, uint16_t channels) override;
	virtual ShaderHandle createShader(const std::string& fileName, const std::string& filePath) override;
	virtual GeometryHandle createGeometry(const BufferLayout& layout, void* vertexData, uint32_t vertexSize, const std::vector<uint16_t>& indices) override;
	virtual RenderableHandle createRenderable(GeometryHandle geometry, MaterialHandle material) override;

	virtual void destroy(CameraHandle handle) override;
	virtual void destroy(FramebufferHandle handle) override;
	virtual void destroy(RenderStateHandle handle) override;
	virtual void destroy(MaterialHandle handle) override;
	virtual void destroy(TextureHandle handle) override;
	virtual void destroy(ShaderHandle handle) override;
	virtual void destroy(GeometryHandle handle) override;
	virtual void destroy(RenderableHandle handle) override;

	virtual void render(CameraHandle camera) override;
	virtual void swapBuffers() override;
	virtual void clear(uint16_t flags, uint16_t width = 0, uint16_t height = 0) override; // 0 size will write to back buffer ratio
	virtual void setClearColor(uint32_t rgba) override;
	virtual void reloadShaders() override;

	virtual void setActiveRenderState(RenderStateHandle renderState) override;
	virtual void setActiveFramebuffer(FramebufferHandle framebuffer) override;// null will write to back buffer
	virtual void setActiveRenderable(RenderableHandle renderable) override;
	virtual void setActiveRenderables(std::vector<RenderableHandle> renderables) override;

	virtual [[nodiscard]] CameraHandle getActiveCamera() override { return mCurrentCamera; }
	virtual [[nodiscard]] RenderStateHandle getActiveRenderState() override { return mCurrentRenderState; }
	virtual [[nodiscard]] RenderableList getActiveRenderables() override { return mCurrentRenderables; }
	virtual [[nodiscard]] BufferList getSharedBuffers() override { return mSharedBuffers; }
	virtual [[nodiscard]] UniformDataList getSharedUniformData() override { return mSharedUniformData; }

	virtual void submitDebugText(uint16_t x, uint16_t y, std::string_view text, ...) override;
	virtual void submitDebugText(uint16_t x, uint16_t y, Color color, std::string_view text, ...) override;
	virtual void submitDebugText(uint16_t x, uint16_t y, Color color, bool right, bool top, std::string_view text, ...) override;

	virtual void submit(GeometryHandle, ShaderHandle shader, CameraHandle camera) override;
	virtual void submit(RenderableHandle renderable, CameraHandle camera) override;
	virtual void submit(RenderableList renderables, CameraHandle camera) override;
	virtual void submit(RenderableHandle renderable, ShaderHandle shader, CameraHandle camera) override;
	virtual void submit(RenderableList renderables, ShaderHandle shader, CameraHandle camera) override;

	virtual void setTexture(ShaderHandle shader, const std::string& uniform, TextureHandle data, uint8_t unit) override;
	virtual void setUniform(ShaderHandle shader, const std::string& uniform, void* data) override;

	virtual CameraSettings getCameraSettings(CameraHandle camera) override;
	virtual float* getCameraProjection(CameraHandle camera) override;
	virtual void setCameraSettings(CameraHandle camera, const CameraSettings& settings) override;

	virtual void setMaterialUniformData(MaterialHandle material, const std::string& name, UniformData::UniformType type, void* data) override;
	virtual void setMaterialTextureData(MaterialHandle material, const std::string& name, TextureHandle texture) override;
	virtual [[nodiscard]] const UniformDataList& getMaterialUniformData(MaterialHandle material) override;
	virtual [[nodiscard]] const TextureDataList& getMaterialTextureData(MaterialHandle material) override;
	virtual [[nodiscard]] const ShaderHandle getMaterialShader(MaterialHandle material) override;

	virtual [[nodiscard]] TextureRef getTextureData(TextureHandle texture) override;
	virtual [[nodiscard]] TextureFormat getTextureFormat(TextureHandle texture) override;

	virtual void setRenderableMaterial(RenderableHandle renderable, MaterialHandle material) override;
	virtual void setRenderableTransform(RenderableHandle renderable, float* matrix) override;
	virtual float* getRenderableTransform(RenderableHandle renderable) override;

	virtual void setSettings(const RenderSettings& settings) override;
	virtual RenderSettings getSettings() override { return mSettings; };

	virtual Stats* getStats() override { return &mStats; };
	virtual RendererRef getRenderer() override { return mRenderer; }
	virtual RenderSystemList getRenderSystems() override { return mRenderSystems; }

private:
	virtual void setRenderStateCount(uint32_t stateCount);
	virtual [[nodiscard]] const uint32_t getRenderStateCount() const { return mRenderStateCount; };
	void updateStats();
	uint8_t colorToAnsi(const Color& color);
	bool setupRenderSystems();
	void setupResetFlags();

private:
	RenderSettings mSettings;
	Stats mStats;

	// Renderer
	RendererRef mRenderer;
	RenderSystemList mRenderSystems;

	// Gfx data
	std::unordered_map<uint16_t, CameraRef> mCameras;
	std::unordered_map<uint16_t, FramebufferRef> mFramebuffers;
	std::unordered_map<uint16_t, RenderStateRef> mRenderStates;
	std::unordered_map<uint16_t, MaterialRef> mMaterials;
	std::unordered_map<uint16_t, TextureRef> mTextures;
	std::unordered_map<uint16_t, ShaderRef> mShaders;
	std::unordered_map<uint16_t, GeometryRef> mGeometries;
	std::unordered_map<uint16_t, RenderableRef> mRenderables;

	// Internal handles
	CameraHandle mCurrentCamera;
	RenderStateHandle mCurrentRenderState;
	RenderableList mCurrentRenderables;

	BufferList mSharedBuffers;
	UniformDataList mSharedUniformData;

	TextureHandle mEmptyTexture; // @todo better way? something is wrong on this texture anyway

	// Backend 
	uint32_t mResetFlags;
	uint32_t mClearColor;
	uint32_t mRenderStateCount;
};

}	// namespace mrender