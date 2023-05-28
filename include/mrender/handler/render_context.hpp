#pragma once

#include <string_view>
#include <vector>
#include <memory>
#include <cstdarg> 
#include <unordered_map>

namespace mrender {

class RenderSystem;
class Renderer;
//class Shader;

struct RenderSettings
{
	std::string_view mRendererName = "none";
	int mResolutionWidth = 1280;
	int mResolutionHeight = 720;
	void* mNativeWindow = nullptr;
	void* mNativeDisplay = nullptr;
	bool mVSync = false;
};

struct RenderStats
{
	uint32_t mNumDrawCalls = 0;
};

class RenderContext
{
public:
	void initialize(const RenderSettings& settings);
	void cleanup();

	void render(const RenderSettings& settings);
	void frame();

	void reset(const int pass, const int width, const int height);
	void reset(const int pass);

	void submitDebugTextOnScreen(uint16_t x, uint16_t y, std::string_view text, ...);

	//void addShader(std::unique_ptr<Shader> shader);
	//void reloadShaders();

	[[nodiscard]] const RenderSettings getSettings() const { return mSettings; }
	[[nodiscard]] const RenderStats getStats() const { return mStats; }

	[[nodiscard]] const std::unique_ptr<Renderer>& getRenderer() const { return mRenderer; };
	[[nodiscard]] const std::vector<std::unique_ptr<RenderSystem>>& getRenderSystems() const { return mRenderSystems; }
	//[[nodiscard]] const std::vector<std::unique_ptr<Shader>>& getShader() const { return mShaders; }

private:
	bool setupRenderSystems();
	void setupResetFlags();

private:
	RenderSettings mSettings;
	RenderStats mStats;

	std::unique_ptr<Renderer> mRenderer;
	std::vector<std::unique_ptr<RenderSystem>> mRenderSystems;
	//std::vector<std::unique_ptr<Shader>> mShaders;

	uint32_t mResetFlags;
};

}	// namespace mrender