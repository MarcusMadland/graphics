#pragma once

#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>

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

// 
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

enum class LightType
{
	Point,
	Spot,
	Directional,
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

	std::shared_ptr<void> mValue;
};

struct LightData // @todo
{
	LightType mType = LightType::Point;

	float mColor[3] = { 0.0f, 0.0f, 0.0f };
	float mIntensity = 1.0f;
	float mPosition[3] = { 0.0f, 0.0f, 0.0f }; // only valid for point+spot lights
	float mDirection[3] = { 0.0f, 0.0f, -1.0f }; //only valid for directional+spot lights
	float mRange = FLT_MAX; //only valid for point+spot lights
	float mInnerConeAngle = 0.0f; //only valid for spot lights
	float mOuterConeAngle = 3.1415926535897932384626433832795f / 4.0f; //only valid for spot lights
};

//
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

//
using UniformDataList = std::unordered_map<std::string, UniformData>;
using TextureDataList = std::unordered_map<std::string, TextureHandle>;
using BufferList = std::unordered_map<std::string, TextureHandle>;
using RenderableList = std::vector<RenderableHandle>;

//
class RenderSystem : public Timable// @todo Simplify class
{
public:
	RenderSystem(const std::string_view& name);

	[[nodiscard]] std::string_view getName() const { return mName; }

	virtual bool init(GfxContext* context) = 0;
	virtual void render(GfxContext* context) = 0;

	virtual BufferList getBuffers(GfxContext* context) = 0;

protected:
	std::string_view mName;
};

class Renderer : public Factory<Renderer> // @todo Simplify class
{
public:
	Renderer(Key) {}

	virtual std::vector<std::shared_ptr<RenderSystem>> setupRenderSystems(GfxContext* context) = 0;
};

//
class GfxContext
{
public:
	virtual CameraHandle createCamera(const CameraSettings& settings) = 0;
	virtual FramebufferHandle createFramebuffer(BufferList buffers) = 0;
	virtual RenderStateHandle createRenderState(uint64_t flags) = 0;
	virtual MaterialHandle createMaterial(ShaderHandle shader) = 0;
	virtual TextureHandle createTexture(TextureFormat format, uint64_t textureFlags, uint16_t width = 0, uint16_t height = 0) = 0;
	virtual TextureHandle createTexture(const uint8_t* data, TextureFormat format, uint64_t textureFlags, uint16_t width, uint16_t height, uint16_t channels) = 0;
	virtual ShaderHandle createShader(const std::string& fileName, const std::string& filePath) = 0;
	virtual GeometryHandle createGeometry(const BufferLayout& layout, void* vertexData, uint32_t vertexSize, const std::vector<uint16_t>& indices) = 0;
	virtual RenderableHandle createRenderable(GeometryHandle geometry, MaterialHandle material) = 0;

	virtual void destroy(CameraHandle handle) = 0;
	virtual void destroy(FramebufferHandle handle) = 0;
	virtual void destroy(RenderStateHandle handle) = 0;
	virtual void destroy(MaterialHandle handle) = 0;
	virtual void destroy(TextureHandle handle) = 0;
	virtual void destroy(ShaderHandle handle) = 0;
	virtual void destroy(GeometryHandle handle) = 0;
	virtual void destroy(RenderableHandle handle) = 0;

	virtual void render(CameraHandle handle) = 0;
	virtual void swapBuffers() = 0;
	virtual void clear(uint16_t flags, uint16_t width = 0, uint16_t height = 0) = 0;
	virtual void setClearColor(uint32_t rgba) = 0;
	virtual void reloadShaders() = 0;

	virtual void setActiveRenderState(RenderStateHandle handle) = 0; 
	virtual void setActiveFramebuffer(FramebufferHandle handle) = 0; // null will write to back buffer
	virtual void setActiveRenderable(RenderableHandle renderable) = 0;
	virtual void setActiveRenderables(RenderableList renderables) = 0;

	virtual [[nodiscard]] CameraHandle getActiveCamera() = 0;
	virtual [[nodiscard]] RenderStateHandle getActiveRenderState() = 0;
	virtual [[nodiscard]] RenderableList getActiveRenderables() = 0;
	virtual [[nodiscard]] BufferList getSharedBuffers() = 0;

	virtual void submitDebugText(uint16_t x, uint16_t y, std::string_view text, ...) = 0;
	virtual void submitDebugText(uint16_t x, uint16_t y, Color color, std::string_view text, ...) = 0;
	virtual void submitDebugText(uint16_t x, uint16_t y, Color color, bool right, bool top, std::string_view text, ...) = 0;
	
	virtual void submit(GeometryHandle, ShaderHandle shaderName, CameraHandle camera) = 0;
	virtual void submit(RenderableHandle renderable, CameraHandle camera) = 0;
	virtual void submit(RenderableList renderables, CameraHandle camera) = 0;
	
	virtual void setTexture(ShaderHandle shader, const std::string& uniform, TextureHandle data, uint8_t unit) = 0;
	virtual void setUniform(ShaderHandle shader, const std::string& uniform, std::shared_ptr<void> data) = 0;

	virtual CameraSettings getCameraSettings(CameraHandle camera) = 0;
	virtual void setCameraSettings(CameraHandle camera, const CameraSettings& settings) = 0;

	virtual void setMaterialUniformData(MaterialHandle material, const std::string& name, UniformData::UniformType type, std::shared_ptr<void> data) = 0;
	virtual void setMaterialTextureData(MaterialHandle material, const std::string& name, TextureHandle texture) = 0;
	virtual [[nodiscard]] const UniformDataList& getMaterialUniformData(MaterialHandle material) = 0;
	virtual [[nodiscard]] const TextureDataList& getMaterialTextureData(MaterialHandle material) = 0;
	virtual [[nodiscard]] const ShaderHandle getMaterialShader(MaterialHandle material) = 0;

	virtual [[nodiscard]] TextureRef getTextureData(TextureHandle texture) = 0;
	virtual [[nodiscard]] TextureFormat getTextureFormat(TextureHandle texture) = 0;

	virtual void setRenderableTransform(RenderableHandle renderable, float* matrix) = 0;
	virtual float* getRenderableTransform(RenderableHandle renderable) = 0;

	virtual void setSettings(const RenderSettings& settings) = 0;
	virtual RenderSettings getSettings() = 0;

};

GfxContext* createGfxContext(const RenderSettings& settings);

}	// namespace mrender