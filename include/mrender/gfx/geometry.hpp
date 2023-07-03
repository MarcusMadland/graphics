#pragma once

#include "mrender/mrender.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

class GeometryImplementation : public Geometry
{
	friend class GfxContextImplementation;

public:
	GeometryImplementation(const BufferLayout& layout, void* vertexData, uint32_t vertexSize, std::vector<uint16_t> indices);

	[[nodiscard]] virtual const uint8_t* getVertexData() const { return mVertexData; };
	[[nodiscard]] virtual const std::vector<uint16_t> getIndexData() const { return mIndexData; }

private:
	bgfx::Attrib::Enum attribToBgfx(const BufferElement::Attrib& attrib);
	bgfx::AttribType::Enum attribTypeToBgfx(const BufferElement::AttribType& attribType);

private:
	bgfx::VertexBufferHandle mVertexBufferHandle;
	bgfx::IndexBufferHandle mIndexBufferHandle;
	std::vector<uint16_t> mIndexData;
	uint8_t* mVertexData = nullptr;
};

}	// namespace mrender