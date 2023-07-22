#pragma once

#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>
#include <variant>

#include "utils/factory.hpp"
#include "utils/timable.hpp"

#define HANDLE(name) struct name { uint16_t idx; }; inline bool isValid(name handle) { return handle.idx != UINT16_MAX; }
#define INVALID_HANDLE { UINT16_MAX }

#define STATIC_IMPL_CAST(T, value) std::static_pointer_cast<T##Implementation>(value)

namespace mrender {

class GfxContext;

HANDLE(CameraHandle)
HANDLE(FramebufferHandle)
HANDLE(RenderStateHandle)
HANDLE(MaterialHandle)
HANDLE(TextureHandle)
HANDLE(ShaderHandle)
HANDLE(GeometryHandle)
HANDLE(RenderableHandle)
HANDLE(LightHandle)

// 
enum class RenderOrder
{
	Default,         
	Sequential,     
	DepthAscending, 
	DepthDescending, 
};

enum class TextureFormat
{
	BC1,
	BC2,
	BC3,
	BC4,
	BC5,
	BC6H,
	BC7,
	ETC1,
	ETC2,
	ETC2A,
	ETC2A1,
	PTC12,
	PTC14,
	PTC12A,
	PTC14A,
	PTC22,
	PTC24,
	ATC,
	ATCE,
	ATCI,
	ASTC4x4,
	ASTC5x5,
	ASTC6x6,
	ASTC8x5,
	ASTC8x6,
	ASTC10x5,

	Unknown,

	R1,
	A8,
	R8,
	R8I,
	R8U,
	R8S,
	R16,
	R16I,
	R16U,
	R16F,
	R16S,
	R32I,
	R32U,
	R32F,
	RG8,
	RG8I,
	RG8U,
	RG8S,
	RG16,
	RG16I,
	RG16U,
	RG16F,
	RG16S,
	RG32I,
	RG32U,
	RG32F,
	RGB8,
	RGB8I,
	RGB8U,
	RGB8S,
	RGB9E5F,
	BGRA8,
	RGBA8,
	RGBA8I,
	RGBA8U,
	RGBA8S,
	RGBA16,
	RGBA16I,
	RGBA16U,
	RGBA16F,
	RGBA16S,
	RGBA32I,
	RGBA32U,
	RGBA32F,
	R5G6B5,
	RGBA4,
	RGB5A1,
	RGB10A2,
	RG11B10F,

	UnknownDepth,

	D16,
	D24,
	D24S8,
	D32,
	D16F,
	D24F,
	D32F,
	D0S8,

	Count
};

enum class Color
{
	White,
	Red,
	Blue,
	Green,
};

struct EnviormentData
{
	float mSunLightDirection[4] = { -5.0f, -5.0f, -2.0f, 0.0f };
	float mSunLightColor[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	// @todo and maybe add camera to this? 
	// @todo and skybox?
};

struct RenderSettings
{
	std::string_view mRendererName = "none";
	int mResolutionWidth = 0;
	int mResolutionHeight = 0;
	void* mNativeWindow = nullptr;
	void* mNativeDisplay = nullptr;
	bool mVSync = false;
	bool mRenderDebugText = true;
};

struct CameraSettings
{
	enum ProjectionType
	{
		Perspective,
		Orthographic,
	} mProjectionType = ProjectionType::Perspective;

	float mFov = 45.0f;
	float mWidth = 16.0f;
	float mHeight = 9.0f;
	float mClipNear = 0.001f;
	float mClipFar = 100.0f;
	float mPosition[3] = { 0.0f, 0.0f, 0.0f }; 
	float mLookAt[3] = { 0.0f, 0.0f, 0.0f }; 
};

struct LightSettings
{
	enum LightType
	{
		Point,
		Spot,
		Directional,
	} mType = LightType::Point;

	float mColor[3] = { 1.0f, 1.0f, 1.0f };
	float mIntensity = 1.0f;
	float mPosition[3] = { 0.0f, 2.0f, 2.0f }; // only valid for point+spot lights
	float mDirection[3] = { 0.0f, 0.0f, -1.0f }; //only valid for directional+spot lights
	float mRange = 2.0f; //only valid for point+spot lights
	float mInnerConeAngle = 0.0f; //only valid for spot lights
	float mOuterConeAngle = 3.1415926535897932384626433832795f / 4.0f; //only valid for spot lights
};

struct BufferElement
{
	enum class AttribType
	{
		Uint8,
		Uint10,
		Int16,
		Half,
		Float,
	} mAttribType;

	enum class Attrib
	{
		Position,
		Normal,
		Tangent,
		Bitangent,
		Color0,
		Color1,
		Color2,
		Color3,
		Indices,
		Weight,
		TexCoord0,
		TexCoord1,
		TexCoord2,
		TexCoord3,
		TexCoord4,
		TexCoord5,
		TexCoord6,
		TexCoord7,
	} mAttrib;
 
	uint8_t mNum;
};
using BufferLayout = std::vector<BufferElement>;

struct UniformData
{
	enum class UniformType
	{
		Vec4,
		Mat3,
		Mat4,
	} mType;

	void* mValue;
};

struct Option
{
	using ValueType = std::variant<float, 
		std::array<float, 2>, 
		std::array<float, 3>, 
		std::array<float, 4>, int, 
		std::array<int, 2>, 
		std::array<int, 3>, 
		std::array<int, 4>, bool>;
	ValueType mValue;

	float mMin = 0.0f;
	float mMax = 1.0f;
};

struct Stats
{
	float mCpuTime;
	float mGpuTime;

	uint32_t mNumDrawCalls;    

	uint32_t mNumCameras;
	uint32_t mNumFramebuffers;
	uint32_t mNumRenderStates;
	uint32_t mNumMaterials;
	uint32_t mNumTextures;
	uint32_t mNumShaders;
	uint32_t mNumGeometries;
	uint32_t mNumRenderables;
	uint32_t mNumLights;

	int64_t mTextureMemoryUsed;  // in MiBs 
};

class Camera {};
using CameraRef = std::shared_ptr<Camera>;

class Framebuffer {};
using FramebufferRef = std::shared_ptr<Framebuffer>;

class RenderState {};
using RenderStateRef = std::shared_ptr<RenderState>;

class Material {};
using MaterialRef = std::shared_ptr<Material>;

class Texture {};
using TextureRef = std::shared_ptr<Texture>;

class Shader {};
using ShaderRef = std::shared_ptr<Shader>;

class Geometry {};
using GeometryRef = std::shared_ptr<Geometry>;

class Renderable {};
using RenderableRef = std::shared_ptr<Renderable>;

class Light {};
using LightRef = std::shared_ptr<Light>;

using UniformDataList = std::unordered_map<std::string, UniformData>;
using TextureDataList = std::unordered_map<std::string, TextureHandle>;
using BufferList = std::unordered_map<std::string, TextureHandle>;
using ShaderList = std::unordered_map<std::string, ShaderHandle>;
using OptionList = std::unordered_map<std::string, Option>;
using RenderableList = std::vector<RenderableHandle>;
using LightList = std::vector<LightHandle>;

class RenderSystem : public Timable// @todo Should we inherit timable in the renderer as well?
{
public:
	RenderSystem(const std::string_view& name);

	[[nodiscard]] std::string_view getName() const { return mName; }

	virtual bool init(GfxContext* context) = 0;
	virtual void render(GfxContext* context) = 0;

protected:
	std::string_view mName;
};
using RenderSystemList = std::vector<std::shared_ptr<RenderSystem>>;

//
class Renderer : public Factory<Renderer> // @todo Simplify class
{
public:
	Renderer(Key) {}

	virtual std::vector<std::shared_ptr<RenderSystem>> setupRenderSystems(GfxContext* context) = 0;
};
using RendererRef = std::shared_ptr<Renderer>;

//
class GfxContext
{
public:
	virtual CameraHandle createCamera(const CameraSettings& settings) = 0;
	virtual FramebufferHandle createFramebuffer(BufferList buffers) = 0;
	virtual RenderStateHandle createRenderState(std::string_view name, uint64_t flags, RenderOrder order = RenderOrder::Default) = 0;
	virtual MaterialHandle createMaterial(ShaderHandle shader) = 0;
	virtual TextureHandle createTexture(TextureFormat format, uint64_t textureFlags, uint16_t width = 0, uint16_t height = 0) = 0;
	virtual TextureHandle createTexture(const uint8_t* data, TextureFormat format, uint64_t textureFlags, uint16_t width, uint16_t height, uint16_t channels) = 0;
	virtual ShaderHandle createShader(const std::string& vertexPath, const std::string& fragmentPath) = 0;
	virtual GeometryHandle createGeometry(const BufferLayout& layout, void* vertexData, uint32_t vertexSize, const std::vector<uint16_t>& indices) = 0;
	virtual RenderableHandle createRenderable(GeometryHandle geometry, MaterialHandle material) = 0;
	virtual LightHandle createLight(const LightSettings& settings) = 0;

	virtual void destroy(CameraHandle handle) = 0;
	virtual void destroy(FramebufferHandle handle) = 0;
	virtual void destroy(RenderStateHandle handle) = 0;
	virtual void destroy(MaterialHandle handle) = 0;
	virtual void destroy(TextureHandle handle) = 0;
	virtual void destroy(ShaderHandle handle) = 0;
	virtual void destroy(GeometryHandle handle) = 0;
	virtual void destroy(RenderableHandle handle) = 0;
	virtual void destroy(LightHandle handle) = 0;

	virtual void render(CameraHandle handle) = 0;
	virtual void swapBuffers() = 0;
	virtual void clear(uint16_t flags, uint16_t width = 0, uint16_t height = 0) = 0;
	virtual void setClearColor(uint32_t rgba) = 0;
	virtual void reloadShaders() = 0;

	virtual void setActiveRenderState(RenderStateHandle handle) = 0; 
	virtual void setActiveFramebuffer(FramebufferHandle handle) = 0; // null will write to back buffer
	virtual void setActiveRenderable(RenderableHandle renderable) = 0;
	virtual void setActiveRenderables(RenderableList renderables) = 0;
	virtual void setActiveLight(LightHandle light) = 0;
	virtual void setActiveLights(LightList lights) = 0;

	virtual void addSystemOption(const std::string& name, Option option) = 0;
	virtual void addSharedBuffer(const std::string& name, TextureHandle texture) = 0;
	virtual void addSharedUniformData(const std::string& name, UniformData data) = 0;

	virtual [[nodiscard]] CameraHandle getActiveCamera() = 0;
	virtual [[nodiscard]] RenderStateHandle getActiveRenderState() = 0;
	virtual [[nodiscard]] ShaderList getActiveShaders() = 0;
	virtual [[nodiscard]] RenderableList getActiveRenderables() = 0;
	virtual [[nodiscard]] LightList getActiveLights() = 0;

	virtual [[nodiscard]] BufferList getSharedBuffers() = 0;
	virtual [[nodiscard]] UniformDataList getSharedUniformData() = 0;

	virtual void submitDebugText(uint16_t x, uint16_t y, std::string_view text, ...) = 0;
	virtual void submitDebugText(uint16_t x, uint16_t y, Color color, std::string_view text, ...) = 0;
	virtual void submitDebugText(uint16_t x, uint16_t y, Color color, bool right, bool top, std::string_view text, ...) = 0;
	
	virtual void submitDebugCube(float* transform, Color color = Color::Red) = 0;

	virtual void submit(GeometryHandle, ShaderHandle shader, CameraHandle camera) = 0;
	virtual void submit(RenderableHandle renderable, CameraHandle camera) = 0;
	virtual void submit(RenderableList renderables, CameraHandle camera) = 0;
	virtual void submit(RenderableHandle renderable, ShaderHandle shader, CameraHandle camera) = 0;
	virtual void submit(RenderableList renderables, ShaderHandle shader, CameraHandle camera) = 0;
	virtual void submit(ShaderHandle shader, uint64_t flags = 0) = 0;
	
	virtual void setTexture(ShaderHandle shader, const std::string& uniform, TextureHandle data, uint8_t unit) = 0;
	virtual void setUniform(ShaderHandle shader, const std::string& uniform, void* data) = 0;

	virtual CameraSettings getCameraSettings(CameraHandle camera) = 0;
	virtual float* getCameraView(CameraHandle camera) = 0;
	virtual float* getCameraProj(CameraHandle camera) = 0;
	virtual float* getCameraViewProj(CameraHandle camera) = 0;
	virtual void setCameraSettings(CameraHandle camera, const CameraSettings& settings) = 0;

	virtual void setMaterialUniformData(MaterialHandle material, const std::string& name, UniformData::UniformType type, void* data) = 0;
	virtual void setMaterialTextureData(MaterialHandle material, const std::string& name, TextureHandle texture) = 0;
	virtual [[nodiscard]] const UniformDataList& getMaterialUniformData(MaterialHandle material) = 0;
	virtual [[nodiscard]] const TextureDataList& getMaterialTextureData(MaterialHandle material) = 0;
	virtual [[nodiscard]] const ShaderHandle getMaterialShader(MaterialHandle material) = 0;

	virtual [[nodiscard]] TextureRef getTextureData(TextureHandle texture) = 0;
	virtual [[nodiscard]] uint16_t getTextureID(TextureHandle texture) = 0;
	virtual [[nodiscard]] TextureFormat getTextureFormat(TextureHandle texture) = 0;
	virtual [[nodiscard]] std::vector<uint8_t> readTexture(TextureHandle texture) = 0;

	virtual [[nodiscard]] std::string getShaderName(ShaderHandle shader) = 0;

	virtual void setRenderableMaterial(RenderableHandle renderable, MaterialHandle material) = 0;
	virtual MaterialHandle getRenderableMaterial(RenderableHandle renderable) = 0;
	virtual void setRenderableTransform(RenderableHandle renderable, float* matrix) = 0;
	virtual float* getRenderableTransform(RenderableHandle renderable) = 0;

	virtual LightSettings getLightSettings(LightHandle light) = 0;
	virtual void setLightSettings(LightHandle light, const LightSettings& settings) = 0;

	virtual void setSettings(const RenderSettings& settings) = 0;
	virtual RenderSettings getSettings() = 0;

	virtual Stats* getStats() = 0;
	virtual RendererRef getRenderer() = 0;
	virtual RenderSystemList getRenderSystems() = 0;

	virtual OptionList getOptions() = 0;
	virtual bool hasOption(const std::string& name) = 0;
	virtual void setOption(const std::string& name, const Option::ValueType& value)= 0;
	virtual const Option& getOption(const std::string& name) = 0;

	template<typename T>
	T getOptionValue(const std::string& name);
};

GfxContext* createGfxContext(const RenderSettings& settings);

template<typename T>
inline T GfxContext::getOptionValue(const std::string& name)
{
	const Option& option = getOption(name);
	if constexpr (std::is_same_v<T, Option::ValueType>) 
	{
		return std::get<T>(option.mValue);
	}
	else 
	{
		return std::get<T>(option.mValue);
	}
}

}	// namespace mrender