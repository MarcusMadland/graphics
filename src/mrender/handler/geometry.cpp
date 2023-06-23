#include "mrender/handler/geometry.hpp"

namespace mrender {

GeometryImplementation::GeometryImplementation(const BufferLayout& layout, void* vertexData, uint32_t vertexSize, std::vector<uint16_t> indices)
	: Geometry(layout, vertexData, vertexSize, indices)
{
	bgfx::VertexLayout bgfxLayout;
	bgfxLayout.begin();
	for (auto& element : layout.mElements)
	{
		bgfxLayout.add(attribToBgfx(element.mAttrib), element.mNum,
			attribTypeToBgfx(element.mAttribType), element.mAttribType == AttribType::Uint8 || element.mAttribType == AttribType::Int16, element.mAttribType == AttribType::Int16);
					// @todo Need a better solution to "normalized" param
	}
	bgfxLayout.end();

    mVertexBufferHandle = bgfx::createVertexBuffer(
        bgfx::makeRef(mVertexData, vertexSize),
		bgfxLayout);
    mIndexBufferHandle = bgfx::createIndexBuffer(
        bgfx::makeRef(mIndexData.data(), static_cast<uint32_t>(mIndexData.size() * sizeof(uint16_t))));
}

bgfx::Attrib::Enum GeometryImplementation::attribToBgfx(const Attrib& attrib)
{
	switch (attrib)
	{
	case Attrib::Position:
		return bgfx::Attrib::Position;

	case Attrib::Normal:
		return bgfx::Attrib::Normal;

	case Attrib::Tangent:
		return bgfx::Attrib::Tangent;

	case Attrib::Bitangent:
		return bgfx::Attrib::Bitangent;

	case Attrib::Color0:
		return bgfx::Attrib::Color0;

	case Attrib::Color1:
		return bgfx::Attrib::Color1;

	case Attrib::Color2:
		return bgfx::Attrib::Color2;

	case Attrib::Color3:
		return bgfx::Attrib::Color3;

	case Attrib::Indices:
		return bgfx::Attrib::Indices;

	case Attrib::Weight:
		return bgfx::Attrib::Weight;

	case Attrib::TexCoord0:
		return bgfx::Attrib::TexCoord0;

	case Attrib::TexCoord1:
		return bgfx::Attrib::TexCoord1;

	case Attrib::TexCoord2:
		return bgfx::Attrib::TexCoord2;

	case Attrib::TexCoord3:
		return bgfx::Attrib::TexCoord3;

	case Attrib::TexCoord4:
		return bgfx::Attrib::TexCoord4;

	case Attrib::TexCoord5:
		return bgfx::Attrib::TexCoord5;

	case Attrib::TexCoord6:
		return bgfx::Attrib::TexCoord6;

	case Attrib::TexCoord7:
		return bgfx::Attrib::TexCoord7;

	default:
		return bgfx::Attrib::Position;
	}
}

bgfx::AttribType::Enum GeometryImplementation::attribTypeToBgfx(const AttribType& attribType)
{
	switch (attribType)
	{
	case AttribType::Uint8:
		return bgfx::AttribType::Uint8;

	case AttribType::Uint10:
		return bgfx::AttribType::Uint10;

	case AttribType::Int16:
		return bgfx::AttribType::Int16;

	case AttribType::Half:
		return bgfx::AttribType::Half;

	case AttribType::Float:
		return bgfx::AttribType::Float;

	default:
		return bgfx::AttribType::Float;
	}
}

}	// namespace mrender