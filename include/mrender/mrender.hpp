#pragma once

#include "utils/factory.hpp"

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
	ProjectionType mProjectionType = ProjectionType::Perspective;
	float mFov = 45.0f;
	float mWidth = 0.0f;
	float mHeight = 0.0f;
	float mClipNear = 0.001f;
	float mClipFar = 100.0f;
	float mPosition[3] = { 0.0f, 0.0f, 0.0f }; 
	float mLookAt[3] = { 0.0f, 0.0f, 0.0f }; 
};

struct BufferElement
{
	AttribType mAttribType;
	uint8_t mNum;
	Attrib mAttrib;
};

struct BufferLayout
{
	std::vector<BufferElement> mElements;
};

class Framebuffer;
class RenderState;
class Texture;
class Shader;
class RenderSystem;
class Renderer;
class Camera;
class Geometry;
class Renderable;

class RenderContext
{
public:
	virtual void initialize(const RenderSettings& settings) = 0;
	virtual void cleanup() = 0;

	virtual void render(const std::shared_ptr<Camera>& camera) = 0;
	virtual void frame() = 0; // @todo Rename to swap buffers

	virtual void setClearColor(uint32_t rgba) = 0;
	
	virtual void writeToBuffers(std::shared_ptr<Framebuffer> framebuffer) = 0;// null will write to back buffer
	virtual void setRenderState(std::shared_ptr<RenderState> renderState) = 0;

	virtual void clear(uint16_t flags, uint16_t width = 0, uint16_t height = 0) = 0;

	virtual void setParameter(const std::string& shader, const std::string& uniform, const std::shared_ptr<Texture>& texture) = 0;

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

	virtual void setRenderStateCount(uint32_t passCount) = 0;

	std::shared_ptr<Shader> createShader();
	std::shared_ptr<RenderState> createRenderState(uint64_t flags);
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
	virtual [[nodiscard]] const uint32_t getRenderStateCount() const = 0;
};

class Framebuffer
{
public:
};

class RenderState 
{
public:
};

class Texture
{
public:
	virtual [[nodiscard]] TextureFormat getFormat() const = 0;
};

class Shader
{
public:
	virtual void loadProgram(char const* fileName, char const* filePath) = 0;
	virtual void reloadProgram() = 0;
};

class RenderSystem // @todo Simplify class
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

class Renderer : public Factory<Renderer> // @todo Simplify class
{
public:
	Renderer(Key) {}

	virtual std::vector<std::shared_ptr<RenderSystem>> setupRenderSystems(RenderContext& context) = 0;
};

class Camera
{
public:
	virtual void recalculate() = 0;

	virtual void setSettings(const CameraSettings& settings) = 0;
	virtual [[nodiscard]] const CameraSettings getSettings() const = 0;
};

class Geometry
{
public:
	[[nodiscard]] virtual const uint8_t* getVertexData() const = 0;
	[[nodiscard]] virtual const std::vector<uint16_t> getIndexData() const = 0;
};

class Renderable
{
public:
	virtual void setTransform(float matrix[16]) = 0; 
	[[nodiscard]] virtual float* getTransform() = 0;

	[[nodiscard]] virtual std::shared_ptr<Geometry> getGeometry() = 0;
	[[nodiscard]] virtual const std::string_view getShader() const = 0;
};

std::shared_ptr<RenderContext> createRenderContext();

}	// namespace mrender