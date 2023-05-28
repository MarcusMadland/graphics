#pragma once

#include "mrender/mrender.hpp"
#include "mrender/handler/shader.hpp"

#include <string_view>
#include <vector>
#include <memory>
#include <cstdarg> 
#include <unordered_map>

namespace mrender {

class RenderContextImplementation : public RenderContext
{
public:
	virtual void initialize(const RenderSettings& settings) override;
	virtual void cleanup() override;

	virtual void render() override;
	virtual void frame() override;

	virtual void reset(const int pass, const int width, const int height) override;
	virtual void reset(const int pass) override;

	virtual void submitDebugTextOnScreen(uint16_t x, uint16_t y, std::string_view text, ...) override;

	virtual void loadShader(char const* fileName, char const* filePath) override;
	virtual void reloadShaders() override;

	virtual void setSettings(const RenderSettings& settings);
	virtual [[nodiscard]] const RenderSettings getSettings() const override { return mSettings; }

	virtual [[nodiscard]] const std::unique_ptr<Renderer>& getRenderer() const override { return mRenderer; };
	virtual [[nodiscard]] const std::vector<std::unique_ptr<RenderSystem>>& getRenderSystems() const override { return mRenderSystems; }
	virtual [[nodiscard]] const std::unordered_map<std::string_view, std::unique_ptr<Shader>>& getShaders() const override { return mShaders; }

private:
	bool setupRenderSystems();
	void setupResetFlags();

private:
	RenderSettings mSettings;

	std::unique_ptr<Renderer> mRenderer;
	std::vector<std::unique_ptr<RenderSystem>> mRenderSystems;
	std::unordered_map<std::string_view, std::unique_ptr<Shader>> mShaders;

	uint32_t mResetFlags;
};

}	// namespace mrender