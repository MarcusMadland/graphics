#pragma once

#include "mrender/mrender.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

class GeometryImplementation : public Geometry
{
	friend class RenderContextImplementation;

public:
	GeometryImplementation(const BufferLayout& layout, void* vertexData, uint32_t vertexSize, std::vector<uint16_t> indices);

private:
	bgfx::Attrib::Enum attribToBgfx(const Attrib& attrib);
	bgfx::AttribType::Enum attribTypeToBgfx(const AttribType& attribType);

private:
	bgfx::VertexBufferHandle mVertexBufferHandle;
	bgfx::IndexBufferHandle mIndexBufferHandle;
};

}	// namespace mrender