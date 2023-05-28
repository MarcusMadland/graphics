#pragma once

#include "core/factory.hpp"

#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>

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

class Shader
{
public:
	virtual void loadProgram(char const* fileName, char const* filePath) = 0;
	virtual void reloadProgram() = 0;
};

class Renderer;
class RenderSystem;

class RenderContext
{
public:
	virtual void initialize(const RenderSettings& settings) = 0;
	virtual void cleanup() = 0;

	virtual void render() = 0;
	virtual void frame() = 0;

	virtual void reset(const int pass, const int width, const int height) = 0;
	virtual void reset(const int pass) = 0;

	virtual void submitDebugTextOnScreen(uint16_t x, uint16_t y, std::string_view text, ...) = 0;

	virtual void loadShader(char const* fileName, char const* filePath) = 0;
	virtual void reloadShaders() = 0;

	virtual void setSettings(const RenderSettings& settings) = 0;
	virtual [[nodiscard]] const RenderSettings getSettings() const = 0;

	virtual [[nodiscard]] const std::unique_ptr<Renderer>& getRenderer() const = 0;
	virtual [[nodiscard]] const std::vector<std::unique_ptr<RenderSystem>>& getRenderSystems() const = 0;
	virtual [[nodiscard]] const std::unordered_map<std::string_view, std::unique_ptr<Shader>>& getShaders() const = 0;
};

class RenderSystem
{
public:
	RenderSystem(const std::string_view& name);

	[[nodiscard]] std::string_view getName() const { return mName; }

	virtual bool init(RenderContext& context) = 0;
	virtual void render(RenderContext& context) = 0;

private:
	std::string_view mName;
};

class Renderer : public Factory<Renderer>
{
public:
	Renderer(Key) {}

	virtual std::vector<std::unique_ptr<RenderSystem>> setupRenderSystems(RenderContext& context) = 0;
};

std::unique_ptr<RenderContext> createRenderContext();
std::unique_ptr<Shader> createShader();

}	// namespace mrender