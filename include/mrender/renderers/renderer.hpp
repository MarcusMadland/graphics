#pragma once

namespace mrender {

struct RenderSettings 
{
	int mResolutionWidth = 1280;
	int mResolutionHeight = 720;
	void* mNativeWindow = nullptr;
	void* mNativeDisplay = nullptr;
	bool mVSync = false;
};

struct TechniqueSettings
{

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

class Renderer
{
public:
	virtual void initialize(RenderContext& context) = 0;
	virtual void render() = 0;
};

}	// namespace mrender