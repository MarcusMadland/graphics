#pragma once

#include <string_view>

namespace mrender {

struct RenderSettings
{
	std::string_view mRendererName = "TestingRenderer";

	int mResolutionWidth = 1280;
	int mResolutionHeight = 720;
	void* mNativeWindow = nullptr;
	void* mNativeDisplay = nullptr;
	bool mVSync = false;
};

class RenderContext
{
public:
	void initialize(const RenderSettings& settings);
	void cleanup();

	void reset(int pass, const int width, const int height);

	RenderSettings& getSettings() { return mSettings; }

private:
	RenderSettings mSettings;
};

}	// namespace mrender