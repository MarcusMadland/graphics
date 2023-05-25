#pragma once

#include <string_view>

namespace mrender {

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
	std::string_view mGraphicsDevice = "none";
	std::string_view mGraphicsAPI = "none";

	uint32_t mNumDrawCalls = 0;
	uint32_t mRuntimeMemory = 0;
};

class RenderContext
{
public:
	void initialize(const RenderSettings& settings);
	void cleanup();

	void reset(int pass, const int width, const int height);

	RenderSettings& getSettings() { return mSettings; }
	const RenderStats* getStats();

private:
	RenderSettings mSettings;
	RenderStats renderStats;
};

}	// namespace mrender