/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#ifndef GRAPHICS_VERTEXDECL_H_HEADER_GUARD
#define GRAPHICS_VERTEXDECL_H_HEADER_GUARD

#include <graphics/graphics.h>
#include <base/readerwriter.h>

namespace graphics
{
	///
	void initAttribTypeSizeTable(RendererType::Enum _type);

	///
	bool isFloat(AttribType::Enum _type);

	/// Returns attribute name.
	const char* getAttribName(Attrib::Enum _attr);

	///
	const char* getAttribNameShort(Attrib::Enum _attr);

	///
	Attrib::Enum idToAttrib(uint16_t id);

	///
	uint16_t attribToId(Attrib::Enum _attr);

	///
	AttribType::Enum idToAttribType(uint16_t id);

	///
	int32_t write(base::WriterI* _writer, const graphics::VertexLayout& _layout, base::Error* _err = NULL);

	///
	int32_t read(base::ReaderI* _reader, graphics::VertexLayout& _layout, base::Error* _err = NULL);

	///
	uint32_t weldVertices(void* _output, const VertexLayout& _layout, const void* _data, uint32_t _num, bool _index32, float _epsilon, base::AllocatorI* _allocator);

} // namespace graphics

#endif // GRAPHICS_VERTEXDECL_H_HEADER_GUARD
