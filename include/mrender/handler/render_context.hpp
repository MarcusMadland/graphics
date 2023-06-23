#pragma once

#include "mrender/mrender.hpp"
#include "mrender/handler/shader.hpp"
#include "mrender/handler/render_state.hpp"

#include <string_view>
#include <vector>
#include <memory>
#include <cstdarg> 
#include <unordered_map>

namespace mrender {

class RenderContextImplementation : public RenderContext
{
	friend class RenderStateImplementation;
	friend class FrameBufferImplementation;
	friend class PostProcessing;
	friend class ShadowMapping;
	friend class GBuffer;

public:
	virtual void initialize(const RenderSettings& settings) override;
	virtual void cleanup() override;

	virtual void render(const std::shared_ptr<Camera>& camera) override;
	virtual void frame() override;

	virtual void setClearColor(uint32_t rgba) override;
	virtual void writeToBuffers(std::shared_ptr<Framebuffer> framebuffer) override;
	virtual void setRenderState(std::shared_ptr<RenderState> renderState) override;

	virtual void clear(uint16_t flags, uint16_t width = 0, uint16_t height = 0) override;

	virtual void setParameter(const std::string_view& shader, const std::string_view& uniform, const std::shared_ptr<Texture>& texture) override;

	virtual void submitDebugTextOnScreen(uint16_t x, uint16_t y, std::string_view text, ...) override;
	virtual void submitDebugTextOnScreen(uint16_t x, uint16_t y, Color color, std::string_view text, ...) override;
	virtual void submitDebugTextOnScreen(uint16_t x, uint16_t y, Color color, bool right, bool top, std::string_view text, ...) override;
	virtual void submit(const std::shared_ptr<Geometry>& geometry, const std::string_view& shaderName, const std::shared_ptr<Camera>& camera) override;
	virtual void submit(const std::shared_ptr<Renderable>& renderable, const std::shared_ptr<Camera>& camera) override;
	virtual void submit(const std::vector<std::shared_ptr<Renderable>>& renderables, const std::shared_ptr<Camera>& camera) override;

	virtual void loadShader(char const* fileName, char const* filePath) override;
	virtual void reloadShaders() override;

	virtual void setSettings(const RenderSettings& settings);
	
	virtual void addRenderable(std::shared_ptr<Renderable> renderable) override;
	virtual void setRenderables(std::vector<std::shared_ptr<Renderable>> renderables) override;

	virtual void addBuffer(const std::string_view& name, std::shared_ptr<Texture> buffer) override;

	virtual [[nodiscard]] const RenderSettings getSettings() const override { return mSettings; }
	virtual [[nodiscard]] const std::shared_ptr<Renderer>& getRenderer() const override { return mRenderer; };
	virtual [[nodiscard]] const std::unordered_map<std::string, std::shared_ptr<Texture>>& getBuffers() const override { return mBuffers; }
	virtual [[nodiscard]] const std::vector<std::shared_ptr<RenderSystem>>& getRenderSystems() const override { return mRenderSystems; }
	virtual [[nodiscard]] const std::unordered_map<std::string, std::shared_ptr<Shader>>& getShaders() const override { return mShaders; }
	virtual [[nodiscard]] const std::vector<std::shared_ptr<Renderable>>& getRenderables() const override { return mRenderables; }
	virtual [[nodiscard]] const std::shared_ptr<Camera>& getCamera() const override { return mCamera; }
	virtual [[nodiscard]] const uint32_t getRenderStateCount() const override { return mRenderStateCount; };

private:
	virtual void setRenderStateCount(uint32_t stateCount) override;
	uint8_t colorToAnsi(const Color& color);
	bool setupRenderSystems();
	void setupResetFlags();

private:
	RenderSettings mSettings;

	std::shared_ptr<Renderer> mRenderer;
	std::vector<std::shared_ptr<RenderSystem>> mRenderSystems;
	std::unordered_map<std::string, std::shared_ptr<Shader>> mShaders;
	std::vector<std::shared_ptr<Renderable>> mRenderables;
	std::shared_ptr<Camera> mCamera;
	std::unordered_map<std::string, std::shared_ptr<Texture>> mBuffers;
	std::shared_ptr<RenderStateImplementation> mRenderState;

	uint32_t mResetFlags;
	uint32_t mClearColor = 0xFF00FFFF;
	uint32_t mRenderStateCount = 0;
};

}	// namespace mrender