#pragma once

#include "core/factory.hpp"

#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>

namespace mrender {

enum class AttribType
{
	Uint8,
	Uint10,
	Int16,
	Half,
	Float,
};

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

enum class ProjectionType
{
	Perspective,
	Orthographic,
};

struct RenderSettings
{
	std::string_view mRendererName = "none";
	int mResolutionWidth = 0;
	int mResolutionHeight = 0;
	void* mNativeWindow = nullptr;
	void* mNativeDisplay = nullptr;
	bool mVSync = false;
};

struct CameraSettings
{
	ProjectionType projectionType = ProjectionType::Perspective;
	float fov = 45.0f;
	float width = 0.0f;
	float height = 0.0f;
	float clipNear = 0.001f;
	float clipFar = 100.0f;
	float postion[3] = { 0.0f, 0.0f, 0.0f }; // x, y, z
	float rotation[3] = { 0.0f, 0.0f, 0.0f }; // pitch, yaw, roll
};

struct BufferElement
{
	AttribType attribType;
	uint8_t num;
	Attrib attrib;
};

struct BufferLayout
{
	std::vector<BufferElement> mElements;
};

class Renderer;
class RenderSystem;
class Renderable;
class Geometry;
class Camera;
class Shader;
class RenderPass;
class Framebuffer;
class Texture;

class RenderContext
{
public:
	virtual void initialize(const RenderSettings& settings) = 0;
	virtual void cleanup() = 0;

	virtual void render(const std::shared_ptr<Camera>& camera) = 0;
	virtual void frame() = 0;

	virtual void setClearColor(uint32_t rgba) = 0;
	virtual void writeToBuffer(const std::string_view& buffer, bool writeToBackBuffer = false) = 0;

	virtual void clear() = 0;

	virtual void setParameter(const std::string_view& shader, const std::string_view& uniform, const std::shared_ptr<Texture>& texture) = 0;

	virtual void submitDebugTextOnScreen(uint16_t x, uint16_t y, std::string_view text, ...) = 0;
	virtual void submitDebugTextOnScreen(uint16_t x, uint16_t y, Color color, std::string_view text, ...) = 0;
	virtual void submitDebugTextOnScreen(uint16_t x, uint16_t y, Color color, bool right, bool top, std::string_view text, ...) = 0;
	virtual void submit(const std::shared_ptr<Geometry>& geometry, const std::string_view& shaderName, const std::shared_ptr<Camera>& camera) = 0;
	virtual void submit(const std::shared_ptr<Renderable>& renderables, const std::shared_ptr<Camera>& camera) = 0;
	virtual void submit(const std::vector<std::shared_ptr<Renderable>>& renderables, const std::shared_ptr<Camera>& camera) = 0;

	virtual void loadShader(char const* fileName, char const* filePath) = 0;
	virtual void reloadShaders() = 0;

	virtual void setSettings(const RenderSettings& settings) = 0;

	virtual void addRenderable(std::shared_ptr<Renderable> renderable) = 0;
	virtual void setRenderables(std::vector<std::shared_ptr<Renderable>> renderables) = 0;

	virtual void addBuffer(const std::string_view& name, std::shared_ptr<Texture> buffer) = 0;

	virtual void setPassCount(uint32_t passCount) = 0;

	std::shared_ptr<Shader> createShader();
	std::shared_ptr<Framebuffer> createFramebuffer(std::vector<std::string> buffers);
	std::shared_ptr<Texture> createTexture(TextureFormat format, uint64_t textureFlags, uint16_t width = 0, uint16_t height = 0);
	std::shared_ptr<Camera> createCamera(const CameraSettings& settings);
	std::shared_ptr<Geometry> createGeometry(const BufferLayout& layout, void* vertexData, uint32_t vertexSize, std::vector<uint16_t> indices);
	std::shared_ptr<Renderable> createRenderable(std::shared_ptr<Geometry> geometry, const std::string_view& shader);
	
	virtual [[nodiscard]] const RenderSettings getSettings() const = 0;
	virtual [[nodiscard]] const std::shared_ptr<Renderer>& getRenderer() const = 0;
	virtual [[nodiscard]] const std::unordered_map<std::string, std::shared_ptr<Texture>>& getBuffers() const = 0;
	virtual [[nodiscard]] const std::vector<std::shared_ptr<RenderSystem>>& getRenderSystems() const = 0;
	virtual [[nodiscard]] const std::unordered_map<std::string, std::shared_ptr<Shader>>& getShaders() const = 0;
	virtual [[nodiscard]] const std::vector<std::shared_ptr<Renderable>>& getRenderables() const = 0;
	virtual [[nodiscard]] const std::shared_ptr<Camera>& getCamera() const = 0;
	virtual [[nodiscard]] const uint32_t getPassCount() const = 0;
};

class Framebuffer
{};

class Texture
{
public:
	Texture(TextureFormat format, uint64_t textureFlags, uint16_t width = 0, uint16_t height = 0) {};

	virtual [[nodiscard]] TextureFormat getFormat() const = 0;
};

class Shader
{
public:
	virtual void loadProgram(char const* fileName, char const* filePath) = 0;
	virtual void reloadProgram() = 0;
};

class RenderSystem
{
public:
	RenderSystem(const std::string_view& name);

	[[nodiscard]] std::string_view getName() const { return mName; }

	virtual bool init(RenderContext& context) = 0;
	virtual void render(RenderContext& context) = 0;

	virtual std::unordered_map<std::string, std::shared_ptr<Texture>> getBuffers(RenderContext& context) = 0;

protected:
	std::string_view mName;
};

class Renderer : public Factory<Renderer>
{
public:
	Renderer(Key) {}

	virtual std::vector<std::shared_ptr<RenderSystem>> setupRenderSystems(RenderContext& context) = 0;
};

class Camera
{
public:
	Camera(const CameraSettings& settings);

	virtual void recalculate() = 0;

	virtual void setSettings(const CameraSettings& settings) = 0;
	virtual [[nodiscard]] const CameraSettings getSettings() const { return mSettings; }

protected:
	CameraSettings mSettings;
};

class Geometry
{
public:
	Geometry(const BufferLayout& layout, void* vertexData, uint32_t vertexSize, std::vector<uint16_t> indices);

	[[nodiscard]] const uint8_t* getVertexData() const { return mVertexData; }
	[[nodiscard]] const std::vector<uint16_t> getIndexData() const { return mIndexData; }

protected:
	uint8_t* mVertexData = nullptr;
	std::vector<uint16_t> mIndexData;
};

class Renderable
{
public:
	Renderable(std::shared_ptr<Geometry> geometry, const std::string_view& shader);

	void setTransform(float matrix[16]) { for (int i = 0; i < 16; i++)  mTransform[i] = matrix[i]; }
	[[nodiscard]] float* getTransform() { return mTransform; }

	[[nodiscard]] std::shared_ptr<Geometry> getGeometry() { return mGeometry; } //mGeometry
	[[nodiscard]] const std::string_view getShader() const { return mShader; }

protected:
	float mTransform[16];
	std::shared_ptr<Geometry> mGeometry = nullptr;
	std::string_view mShader;
};

std::shared_ptr<RenderContext> createRenderContext();

}	// namespace mrender