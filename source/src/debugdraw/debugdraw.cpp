/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include <graphics/graphics.h>
#include <graphics/embedded_shader.h>
#include "debugdraw.h"

#include <base/debug.h>
#include <base/mutex.h>
#include <base/math.h>
#include <base/sort.h>
#include <base/uint32_t.h>
#include <base/handlealloc.h>

#ifndef DEBUG_DRAW_CONFIG_MAX_GEOMETRY
#	define DEBUG_DRAW_CONFIG_MAX_GEOMETRY 256
#endif // DEBUG_DRAW_CONFIG_MAX_GEOMETRY

struct DebugVertex
{
	float m_x;
	float m_y;
	float m_z;
	float m_len;
	uint32_t m_abgr;

	static void init()
	{
		ms_layout
			.begin()
			.add(graphics::Attrib::Position,  3, graphics::AttribType::Float)
			.add(graphics::Attrib::TexCoord0, 1, graphics::AttribType::Float)
			.add(graphics::Attrib::Color0,    4, graphics::AttribType::Uint8, true)
			.end();
	}

	static graphics::VertexLayout ms_layout;
};

graphics::VertexLayout DebugVertex::ms_layout;

struct DebugUvVertex
{
	float m_x;
	float m_y;
	float m_z;
	float m_u;
	float m_v;
	uint32_t m_abgr;

	static void init()
	{
		ms_layout
			.begin()
			.add(graphics::Attrib::Position,  3, graphics::AttribType::Float)
			.add(graphics::Attrib::TexCoord0, 2, graphics::AttribType::Float)
			.add(graphics::Attrib::Color0,    4, graphics::AttribType::Uint8, true)
			.end();
	}

	static graphics::VertexLayout ms_layout;
};

graphics::VertexLayout DebugUvVertex::ms_layout;

struct DebugShapeVertex
{
	float m_x;
	float m_y;
	float m_z;
	uint8_t m_indices[4];

	static void init()
	{
		ms_layout
			.begin()
			.add(graphics::Attrib::Position, 3, graphics::AttribType::Float)
			.add(graphics::Attrib::Indices,  4, graphics::AttribType::Uint8)
			.end();
	}

	static graphics::VertexLayout ms_layout;
};

graphics::VertexLayout DebugShapeVertex::ms_layout;

struct DebugMeshVertex
{
	float m_x;
	float m_y;
	float m_z;

	static void init()
	{
		ms_layout
			.begin()
			.add(graphics::Attrib::Position, 3, graphics::AttribType::Float)
			.end();
	}

	static graphics::VertexLayout ms_layout;
};

graphics::VertexLayout DebugMeshVertex::ms_layout;

static DebugShapeVertex s_quadVertices[4] =
{
	{-1.0f, 0.0f,  1.0f, { 0, 0, 0, 0 } },
	{ 1.0f, 0.0f,  1.0f, { 0, 0, 0, 0 } },
	{-1.0f, 0.0f, -1.0f, { 0, 0, 0, 0 } },
	{ 1.0f, 0.0f, -1.0f, { 0, 0, 0, 0 } },

};

static const uint16_t s_quadIndices[6] =
{
	0, 1, 2,
	1, 3, 2,
};

static DebugShapeVertex s_cubeVertices[8] =
{
	{-1.0f,  1.0f,  1.0f, { 0, 0, 0, 0 } },
	{ 1.0f,  1.0f,  1.0f, { 0, 0, 0, 0 } },
	{-1.0f, -1.0f,  1.0f, { 0, 0, 0, 0 } },
	{ 1.0f, -1.0f,  1.0f, { 0, 0, 0, 0 } },
	{-1.0f,  1.0f, -1.0f, { 0, 0, 0, 0 } },
	{ 1.0f,  1.0f, -1.0f, { 0, 0, 0, 0 } },
	{-1.0f, -1.0f, -1.0f, { 0, 0, 0, 0 } },
	{ 1.0f, -1.0f, -1.0f, { 0, 0, 0, 0 } },
};

static const uint16_t s_cubeIndices[36] =
{
	0, 1, 2, // 0
	1, 3, 2,
	4, 6, 5, // 2
	5, 6, 7,
	0, 2, 4, // 4
	4, 2, 6,
	1, 5, 3, // 6
	5, 7, 3,
	0, 4, 1, // 8
	4, 5, 1,
	2, 3, 6, // 10
	6, 3, 7,
};

static const uint8_t s_circleLod[] =
{
	37,
	29,
	23,
	17,
	11,
};

static uint8_t getCircleLod(uint8_t _lod)
{
	_lod = _lod > BASE_COUNTOF(s_circleLod)-1 ? BASE_COUNTOF(s_circleLod)-1 : _lod;
	return s_circleLod[_lod];
}

static void circle(float* _out, float _angle)
{
	float sa = base::sin(_angle);
	float ca = base::cos(_angle);
	_out[0] = sa;
	_out[1] = ca;
}

static void squircle(float* _out, float _angle)
{
	float sa = base::sin(_angle);
	float ca = base::cos(_angle);
	_out[0] = base::sqrt(base::abs(sa) ) * base::sign(sa);
	_out[1] = base::sqrt(base::abs(ca) ) * base::sign(ca);
}

uint32_t genSphere(uint8_t _subdiv0, void* _pos0 = NULL, uint16_t _posStride0 = 0, void* _normals0 = NULL, uint16_t _normalStride0 = 0)
{
	if (NULL != _pos0)
	{
		struct Gen
		{
			Gen(void* _pos, uint16_t _posStride, void* _normals, uint16_t _normalStride, uint8_t _subdiv)
				: m_pos( (uint8_t*)_pos)
				, m_normals( (uint8_t*)_normals)
				, m_posStride(_posStride)
				, m_normalStride(_normalStride)
			{
				static const float scale = 1.0f;
				static const float golden = 1.6180339887f;
				static const float len = base::sqrt(golden*golden + 1.0f);
				static const float ss = 1.0f/len * scale;
				static const float ll = ss*golden;

				static const base::Vec3 vv[] =
				{
					{ -ll, 0.0f, -ss },
					{  ll, 0.0f, -ss },
					{  ll, 0.0f,  ss },
					{ -ll, 0.0f,  ss },

					{ -ss,  ll, 0.0f },
					{  ss,  ll, 0.0f },
					{  ss, -ll, 0.0f },
					{ -ss, -ll, 0.0f },

					{ 0.0f, -ss,  ll },
					{ 0.0f,  ss,  ll },
					{ 0.0f,  ss, -ll },
					{ 0.0f, -ss, -ll },
				};

				m_numVertices = 0;

				triangle(vv[ 0], vv[ 4], vv[ 3], scale, _subdiv);
				triangle(vv[ 0], vv[10], vv[ 4], scale, _subdiv);
				triangle(vv[ 4], vv[10], vv[ 5], scale, _subdiv);
				triangle(vv[ 5], vv[10], vv[ 1], scale, _subdiv);
				triangle(vv[ 5], vv[ 1], vv[ 2], scale, _subdiv);
				triangle(vv[ 5], vv[ 2], vv[ 9], scale, _subdiv);
				triangle(vv[ 5], vv[ 9], vv[ 4], scale, _subdiv);
				triangle(vv[ 3], vv[ 4], vv[ 9], scale, _subdiv);

				triangle(vv[ 0], vv[ 3], vv[ 7], scale, _subdiv);
				triangle(vv[ 0], vv[ 7], vv[11], scale, _subdiv);
				triangle(vv[11], vv[ 7], vv[ 6], scale, _subdiv);
				triangle(vv[11], vv[ 6], vv[ 1], scale, _subdiv);
				triangle(vv[ 1], vv[ 6], vv[ 2], scale, _subdiv);
				triangle(vv[ 2], vv[ 6], vv[ 8], scale, _subdiv);
				triangle(vv[ 8], vv[ 6], vv[ 7], scale, _subdiv);
				triangle(vv[ 8], vv[ 7], vv[ 3], scale, _subdiv);

				triangle(vv[ 0], vv[11], vv[10], scale, _subdiv);
				triangle(vv[ 1], vv[10], vv[11], scale, _subdiv);
				triangle(vv[ 2], vv[ 8], vv[ 9], scale, _subdiv);
				triangle(vv[ 3], vv[ 9], vv[ 8], scale, _subdiv);
			}

			void addVert(const base::Vec3& _v)
			{
				base::store(m_pos, _v);
				m_pos += m_posStride;

				if (NULL != m_normals)
				{
					const base::Vec3 normal = base::normalize(_v);
					base::store(m_normals, normal);

					m_normals += m_normalStride;
				}

				m_numVertices++;
			}

			void triangle(const base::Vec3& _v0, const base::Vec3& _v1, const base::Vec3& _v2, float _scale, uint8_t _subdiv)
			{
				if (0 == _subdiv)
				{
					addVert(_v0);
					addVert(_v1);
					addVert(_v2);
				}
				else
				{
					const base::Vec3 v01 = base::mul(base::normalize(base::add(_v0, _v1) ), _scale);
					const base::Vec3 v12 = base::mul(base::normalize(base::add(_v1, _v2) ), _scale);
					const base::Vec3 v20 = base::mul(base::normalize(base::add(_v2, _v0) ), _scale);

					--_subdiv;
					triangle(_v0, v01, v20, _scale, _subdiv);
					triangle(_v1, v12, v01, _scale, _subdiv);
					triangle(_v2, v20, v12, _scale, _subdiv);
					triangle(v01, v12, v20, _scale, _subdiv);
				}
			}

			uint8_t* m_pos;
			uint8_t* m_normals;
			uint16_t m_posStride;
			uint16_t m_normalStride;
			uint32_t m_numVertices;

		} gen(_pos0, _posStride0, _normals0, _normalStride0, _subdiv0);
	}

	uint32_t numVertices = 20*3*base::uint32_max(1, (uint32_t)base::pow(4.0f, _subdiv0) );
	return numVertices;
}

base::Vec3 getPoint(graphics::Axis::Enum _axis, float _x, float _y)
{
	switch (_axis)
	{
		case graphics::Axis::X: return { 0.0f,   _x,   _y };
		case graphics::Axis::Y: return {   _y, 0.0f,   _x };
		default: break;
	}

	return { _x, _y, 0.0f };
}

#include "vs_debugdraw_lines.bin.h"
#include "fs_debugdraw_lines.bin.h"
#include "vs_debugdraw_lines_stipple.bin.h"
#include "fs_debugdraw_lines_stipple.bin.h"
#include "vs_debugdraw_fill.bin.h"
#include "vs_debugdraw_fill_mesh.bin.h"
#include "fs_debugdraw_fill.bin.h"
#include "vs_debugdraw_fill_lit.bin.h"
#include "vs_debugdraw_fill_lit_mesh.bin.h"
#include "fs_debugdraw_fill_lit.bin.h"
#include "vs_debugdraw_fill_texture.bin.h"
#include "fs_debugdraw_fill_texture.bin.h"

static const graphics::EmbeddedShader s_embeddedShaders[] =
{
	GRAPHICS_EMBEDDED_SHADER(vs_debugdraw_lines),
	GRAPHICS_EMBEDDED_SHADER(fs_debugdraw_lines),
	GRAPHICS_EMBEDDED_SHADER(vs_debugdraw_lines_stipple),
	GRAPHICS_EMBEDDED_SHADER(fs_debugdraw_lines_stipple),
	GRAPHICS_EMBEDDED_SHADER(vs_debugdraw_fill),
	GRAPHICS_EMBEDDED_SHADER(vs_debugdraw_fill_mesh),
	GRAPHICS_EMBEDDED_SHADER(fs_debugdraw_fill),
	GRAPHICS_EMBEDDED_SHADER(vs_debugdraw_fill_lit),
	GRAPHICS_EMBEDDED_SHADER(vs_debugdraw_fill_lit_mesh),
	GRAPHICS_EMBEDDED_SHADER(fs_debugdraw_fill_lit),
	GRAPHICS_EMBEDDED_SHADER(vs_debugdraw_fill_texture),
	GRAPHICS_EMBEDDED_SHADER(fs_debugdraw_fill_texture),

	GRAPHICS_EMBEDDED_SHADER_END()
};

#define SPRITE_TEXTURE_SIZE 1024

struct Attrib
{
	uint64_t m_state;
	float    m_offset;
	float    m_scale;
	float    m_spin;
	uint32_t m_abgr;
	bool     m_stipple;
	bool     m_wireframe;
	uint8_t  m_lod;
};

struct Program
{
	enum Enum
	{
		Lines,
		LinesStipple,
		Fill,
		FillMesh,
		FillLit,
		FillLitMesh,
		FillTexture,

		Count
	};
};

struct DebugMesh
{
	enum Enum
	{
		Sphere0,
		Sphere1,
		Sphere2,
		Sphere3,

		Cone0,
		Cone1,
		Cone2,
		Cone3,

		Cylinder0,
		Cylinder1,
		Cylinder2,
		Cylinder3,

		Capsule0,
		Capsule1,
		Capsule2,
		Capsule3,

		Quad,

		Cube,

		Count,

		SphereMaxLod   = Sphere3   - Sphere0,
		ConeMaxLod     = Cone3     - Cone0,
		CylinderMaxLod = Cylinder3 - Cylinder0,
		CapsuleMaxLod  = Capsule3  - Capsule0,
	};

	uint32_t m_startVertex;
	uint32_t m_numVertices;
	uint32_t m_startIndex[2];
	uint32_t m_numIndices[2];
};

struct DebugDrawShared
{
	void init(base::AllocatorI* _allocator)
	{
		if (NULL == _allocator)
		{
			static base::DefaultAllocator allocator;
			m_allocator = &allocator;
		}
		else
		{
			m_allocator = _allocator;
		}

		DebugVertex::init();
		DebugUvVertex::init();
		DebugShapeVertex::init();
		DebugMeshVertex::init();

		graphics::RendererType::Enum type = graphics::getRendererType();

		m_program[Program::Lines] = graphics::createProgram(
			  graphics::createEmbeddedShader(s_embeddedShaders, type, "vs_debugdraw_lines")
			, graphics::createEmbeddedShader(s_embeddedShaders, type, "fs_debugdraw_lines")
			, true
			);

		m_program[Program::LinesStipple] = graphics::createProgram(
			  graphics::createEmbeddedShader(s_embeddedShaders, type, "vs_debugdraw_lines_stipple")
			, graphics::createEmbeddedShader(s_embeddedShaders, type, "fs_debugdraw_lines_stipple")
			, true
			);

		m_program[Program::Fill] = graphics::createProgram(
			  graphics::createEmbeddedShader(s_embeddedShaders, type, "vs_debugdraw_fill")
			, graphics::createEmbeddedShader(s_embeddedShaders, type, "fs_debugdraw_fill")
			, true
			);

		m_program[Program::FillMesh] = graphics::createProgram(
			  graphics::createEmbeddedShader(s_embeddedShaders, type, "vs_debugdraw_fill_mesh")
			, graphics::createEmbeddedShader(s_embeddedShaders, type, "fs_debugdraw_fill")
			, true
			);

		m_program[Program::FillLit] = graphics::createProgram(
			  graphics::createEmbeddedShader(s_embeddedShaders, type, "vs_debugdraw_fill_lit")
			, graphics::createEmbeddedShader(s_embeddedShaders, type, "fs_debugdraw_fill_lit")
			, true
			);

		m_program[Program::FillLitMesh] = graphics::createProgram(
			  graphics::createEmbeddedShader(s_embeddedShaders, type, "vs_debugdraw_fill_lit_mesh")
			, graphics::createEmbeddedShader(s_embeddedShaders, type, "fs_debugdraw_fill_lit")
			, true
			);

		m_program[Program::FillTexture] = graphics::createProgram(
			  graphics::createEmbeddedShader(s_embeddedShaders, type, "vs_debugdraw_fill_texture")
			, graphics::createEmbeddedShader(s_embeddedShaders, type, "fs_debugdraw_fill_texture")
			, true
			);

		u_params   = graphics::createUniform("u_params",   graphics::UniformType::Vec4, 4);
		s_texColor = graphics::createUniform("s_texColor", graphics::UniformType::Sampler);
		m_texture  = graphics::createTexture2D(SPRITE_TEXTURE_SIZE, SPRITE_TEXTURE_SIZE, false, 1, graphics::TextureFormat::BGRA8);

		void* vertices[DebugMesh::Count] = {};
		uint16_t* indices[DebugMesh::Count] = {};
		uint16_t stride = DebugShapeVertex::ms_layout.getStride();

		uint32_t startVertex = 0;
		uint32_t startIndex  = 0;

		for (uint32_t mesh = 0; mesh < 4; ++mesh)
		{
			DebugMesh::Enum id = DebugMesh::Enum(DebugMesh::Sphere0+mesh);

			const uint8_t  tess = uint8_t(3-mesh);
			const uint32_t numVertices = genSphere(tess);
			const uint32_t numIndices  = numVertices;

			vertices[id] = base::alloc(m_allocator, numVertices*stride);
			base::memSet(vertices[id], 0, numVertices*stride);
			genSphere(tess, vertices[id], stride);

			uint16_t* trilist = (uint16_t*)base::alloc(m_allocator, numIndices*sizeof(uint16_t) );
			for (uint32_t ii = 0; ii < numIndices; ++ii)
			{
				trilist[ii] = uint16_t(ii);
			}

			uint32_t numLineListIndices = graphics::topologyConvert(
				  graphics::TopologyConvert::TriListToLineList
				, NULL
				, 0
				, trilist
				, numIndices
				, false
				);
			indices[id] = (uint16_t*)base::alloc(m_allocator, (numIndices + numLineListIndices)*sizeof(uint16_t) );
			uint16_t* indicesOut = indices[id];
			base::memCopy(indicesOut, trilist, numIndices*sizeof(uint16_t) );

			graphics::topologyConvert(
				  graphics::TopologyConvert::TriListToLineList
				, &indicesOut[numIndices]
				, numLineListIndices*sizeof(uint16_t)
				, trilist
				, numIndices
				, false
				);

			m_mesh[id].m_startVertex = startVertex;
			m_mesh[id].m_numVertices = numVertices;
			m_mesh[id].m_startIndex[0] = startIndex;
			m_mesh[id].m_numIndices[0] = numIndices;
			m_mesh[id].m_startIndex[1] = startIndex+numIndices;
			m_mesh[id].m_numIndices[1] = numLineListIndices;

			startVertex += numVertices;
			startIndex  += numIndices + numLineListIndices;

			base::free(m_allocator, trilist);
		}

		for (uint32_t mesh = 0; mesh < 4; ++mesh)
		{
			DebugMesh::Enum id = DebugMesh::Enum(DebugMesh::Cone0+mesh);

			const uint32_t num = getCircleLod(uint8_t(mesh) );
			const float step = base::kPi * 2.0f / num;

			const uint32_t numVertices = num+1;
			const uint32_t numIndices  = num*6;
			const uint32_t numLineListIndices = num*4;

			vertices[id] = base::alloc(m_allocator, numVertices*stride);
			indices[id]  = (uint16_t*)base::alloc(m_allocator, (numIndices + numLineListIndices)*sizeof(uint16_t) );
			base::memSet(indices[id], 0, (numIndices + numLineListIndices)*sizeof(uint16_t) );

			DebugShapeVertex* vertex = (DebugShapeVertex*)vertices[id];
			uint16_t* index = indices[id];

			vertex[num].m_x = 0.0f;
			vertex[num].m_y = 0.0f;
			vertex[num].m_z = 0.0f;
			vertex[num].m_indices[0] = 1;

			for (uint32_t ii = 0; ii < num; ++ii)
			{
				const float angle = step * ii;

				float xy[2];
				circle(xy, angle);

				vertex[ii].m_x = xy[1];
				vertex[ii].m_y = 0.0f;
				vertex[ii].m_z = xy[0];
				vertex[ii].m_indices[0] = 0;

				index[ii*3+0] = uint16_t(num);
				index[ii*3+1] = uint16_t( (ii+1)%num);
				index[ii*3+2] = uint16_t(ii);

				index[num*3+ii*3+0] = 0;
				index[num*3+ii*3+1] = uint16_t(ii);
				index[num*3+ii*3+2] = uint16_t( (ii+1)%num);

				index[numIndices+ii*2+0] = uint16_t(ii);
				index[numIndices+ii*2+1] = uint16_t(num);

				index[numIndices+num*2+ii*2+0] = uint16_t(ii);
				index[numIndices+num*2+ii*2+1] = uint16_t( (ii+1)%num);
			}

			m_mesh[id].m_startVertex = startVertex;
			m_mesh[id].m_numVertices = numVertices;
			m_mesh[id].m_startIndex[0] = startIndex;
			m_mesh[id].m_numIndices[0] = numIndices;
			m_mesh[id].m_startIndex[1] = startIndex+numIndices;
			m_mesh[id].m_numIndices[1] = numLineListIndices;

			startVertex += numVertices;
			startIndex  += numIndices + numLineListIndices;
		}

		for (uint32_t mesh = 0; mesh < 4; ++mesh)
		{
			DebugMesh::Enum id = DebugMesh::Enum(DebugMesh::Cylinder0+mesh);

			const uint32_t num = getCircleLod(uint8_t(mesh) );
			const float step = base::kPi * 2.0f / num;

			const uint32_t numVertices = num*2;
			const uint32_t numIndices  = num*12;
			const uint32_t numLineListIndices = num*6;

			vertices[id] = base::alloc(m_allocator, numVertices*stride);
			indices[id]  = (uint16_t*)base::alloc(m_allocator, (numIndices + numLineListIndices)*sizeof(uint16_t) );
			base::memSet(indices[id], 0, (numIndices + numLineListIndices)*sizeof(uint16_t) );

			DebugShapeVertex* vertex = (DebugShapeVertex*)vertices[id];
			uint16_t* index = indices[id];

			for (uint32_t ii = 0; ii < num; ++ii)
			{
				const float angle = step * ii;

				float xy[2];
				circle(xy, angle);

				vertex[ii].m_x = xy[1];
				vertex[ii].m_y = 0.0f;
				vertex[ii].m_z = xy[0];
				vertex[ii].m_indices[0] = 0;

				vertex[ii+num].m_x = xy[1];
				vertex[ii+num].m_y = 0.0f;
				vertex[ii+num].m_z = xy[0];
				vertex[ii+num].m_indices[0] = 1;

				index[ii*6+0] = uint16_t(ii+num);
				index[ii*6+1] = uint16_t( (ii+1)%num);
				index[ii*6+2] = uint16_t(ii);
				index[ii*6+3] = uint16_t(ii+num);
				index[ii*6+4] = uint16_t( (ii+1)%num+num);
				index[ii*6+5] = uint16_t( (ii+1)%num);

				index[num*6+ii*6+0] = uint16_t(0);
				index[num*6+ii*6+1] = uint16_t(ii);
				index[num*6+ii*6+2] = uint16_t( (ii+1)%num);
				index[num*6+ii*6+3] = uint16_t(num);
				index[num*6+ii*6+4] = uint16_t( (ii+1)%num+num);
				index[num*6+ii*6+5] = uint16_t(ii+num);

				index[numIndices+ii*2+0] = uint16_t(ii);
				index[numIndices+ii*2+1] = uint16_t(ii+num);

				index[numIndices+num*2+ii*2+0] = uint16_t(ii);
				index[numIndices+num*2+ii*2+1] = uint16_t( (ii+1)%num);

				index[numIndices+num*4+ii*2+0] = uint16_t(num + ii);
				index[numIndices+num*4+ii*2+1] = uint16_t(num + (ii+1)%num);
			}

			m_mesh[id].m_startVertex = startVertex;
			m_mesh[id].m_numVertices = numVertices;
			m_mesh[id].m_startIndex[0] = startIndex;
			m_mesh[id].m_numIndices[0] = numIndices;
			m_mesh[id].m_startIndex[1] = startIndex+numIndices;
			m_mesh[id].m_numIndices[1] = numLineListIndices;

			startVertex += numVertices;
			startIndex  += numIndices + numLineListIndices;
		}

		for (uint32_t mesh = 0; mesh < 4; ++mesh)
		{
			DebugMesh::Enum id = DebugMesh::Enum(DebugMesh::Capsule0+mesh);

			const uint32_t num = getCircleLod(uint8_t(mesh) );
			const float step = base::kPi * 2.0f / num;

			const uint32_t numVertices = num*2;
			const uint32_t numIndices  = num*6;
			const uint32_t numLineListIndices = num*6;

			vertices[id] = base::alloc(m_allocator, numVertices*stride);
			indices[id]  = (uint16_t*)base::alloc(m_allocator, (numIndices + numLineListIndices)*sizeof(uint16_t) );
			base::memSet(indices[id], 0, (numIndices + numLineListIndices)*sizeof(uint16_t) );

			DebugShapeVertex* vertex = (DebugShapeVertex*)vertices[id];
			uint16_t* index = indices[id];

			for (uint32_t ii = 0; ii < num; ++ii)
			{
				const float angle = step * ii;

				float xy[2];
				circle(xy, angle);

				vertex[ii].m_x = xy[1];
				vertex[ii].m_y = 0.0f;
				vertex[ii].m_z = xy[0];
				vertex[ii].m_indices[0] = 0;

				vertex[ii+num].m_x = xy[1];
				vertex[ii+num].m_y = 0.0f;
				vertex[ii+num].m_z = xy[0];
				vertex[ii+num].m_indices[0] = 1;

				index[ii*6+0] = uint16_t(ii+num);
				index[ii*6+1] = uint16_t( (ii+1)%num);
				index[ii*6+2] = uint16_t(ii);
				index[ii*6+3] = uint16_t(ii+num);
				index[ii*6+4] = uint16_t( (ii+1)%num+num);
				index[ii*6+5] = uint16_t( (ii+1)%num);

//				index[num*6+ii*6+0] = uint16_t(0);
//				index[num*6+ii*6+1] = uint16_t(ii);
//				index[num*6+ii*6+2] = uint16_t( (ii+1)%num);
//				index[num*6+ii*6+3] = uint16_t(num);
//				index[num*6+ii*6+4] = uint16_t( (ii+1)%num+num);
//				index[num*6+ii*6+5] = uint16_t(ii+num);

				index[numIndices+ii*2+0] = uint16_t(ii);
				index[numIndices+ii*2+1] = uint16_t(ii+num);

				index[numIndices+num*2+ii*2+0] = uint16_t(ii);
				index[numIndices+num*2+ii*2+1] = uint16_t( (ii+1)%num);

				index[numIndices+num*4+ii*2+0] = uint16_t(num + ii);
				index[numIndices+num*4+ii*2+1] = uint16_t(num + (ii+1)%num);
			}

			m_mesh[id].m_startVertex = startVertex;
			m_mesh[id].m_numVertices = numVertices;
			m_mesh[id].m_startIndex[0] = startIndex;
			m_mesh[id].m_numIndices[0] = numIndices;
			m_mesh[id].m_startIndex[1] = startIndex+numIndices;
			m_mesh[id].m_numIndices[1] = numLineListIndices;

			startVertex += numVertices;
			startIndex  += numIndices + numLineListIndices;
		}

		m_mesh[DebugMesh::Quad].m_startVertex = startVertex;
		m_mesh[DebugMesh::Quad].m_numVertices = BASE_COUNTOF(s_quadVertices);
		m_mesh[DebugMesh::Quad].m_startIndex[0] = startIndex;
		m_mesh[DebugMesh::Quad].m_numIndices[0] = BASE_COUNTOF(s_quadIndices);
		m_mesh[DebugMesh::Quad].m_startIndex[1] = 0;
		m_mesh[DebugMesh::Quad].m_numIndices[1] = 0;
		startVertex += BASE_COUNTOF(s_quadVertices);
		startIndex  += BASE_COUNTOF(s_quadIndices);

		m_mesh[DebugMesh::Cube].m_startVertex = startVertex;
		m_mesh[DebugMesh::Cube].m_numVertices = BASE_COUNTOF(s_cubeVertices);
		m_mesh[DebugMesh::Cube].m_startIndex[0] = startIndex;
		m_mesh[DebugMesh::Cube].m_numIndices[0] = BASE_COUNTOF(s_cubeIndices);
		m_mesh[DebugMesh::Cube].m_startIndex[1] = 0;
		m_mesh[DebugMesh::Cube].m_numIndices[1] = 0;
		startVertex += m_mesh[DebugMesh::Cube].m_numVertices;
		startIndex  += m_mesh[DebugMesh::Cube].m_numIndices[0];

		const graphics::Memory* vb = graphics::alloc(startVertex*stride);
		const graphics::Memory* ib = graphics::alloc(startIndex*sizeof(uint16_t) );

		for (uint32_t mesh = DebugMesh::Sphere0; mesh < DebugMesh::Quad; ++mesh)
		{
			DebugMesh::Enum id = DebugMesh::Enum(mesh);
			base::memCopy(&vb->data[m_mesh[id].m_startVertex * stride]
				 , vertices[id]
				 , m_mesh[id].m_numVertices*stride
				 );

			base::memCopy(&ib->data[m_mesh[id].m_startIndex[0] * sizeof(uint16_t)]
				 , indices[id]
				 , (m_mesh[id].m_numIndices[0]+m_mesh[id].m_numIndices[1])*sizeof(uint16_t)
				 );

			base::free(m_allocator, vertices[id]);
			base::free(m_allocator, indices[id]);
		}

		base::memCopy(&vb->data[m_mesh[DebugMesh::Quad].m_startVertex * stride]
			, s_quadVertices
			, sizeof(s_quadVertices)
			);

		base::memCopy(&ib->data[m_mesh[DebugMesh::Quad].m_startIndex[0] * sizeof(uint16_t)]
			, s_quadIndices
			, sizeof(s_quadIndices)
			);

		base::memCopy(&vb->data[m_mesh[DebugMesh::Cube].m_startVertex * stride]
			, s_cubeVertices
			, sizeof(s_cubeVertices)
			);

		base::memCopy(&ib->data[m_mesh[DebugMesh::Cube].m_startIndex[0] * sizeof(uint16_t)]
			, s_cubeIndices
			, sizeof(s_cubeIndices)
			);

		m_vbh = graphics::createVertexBuffer(vb, DebugShapeVertex::ms_layout);
		m_ibh = graphics::createIndexBuffer(ib);
	}

	void shutdown()
	{
		graphics::destroy(m_ibh);
		graphics::destroy(m_vbh);
		for (uint32_t ii = 0; ii < Program::Count; ++ii)
		{
			graphics::destroy(m_program[ii]);
		}
		graphics::destroy(u_params);
		graphics::destroy(s_texColor);
		graphics::destroy(m_texture);
	}

	base::AllocatorI* m_allocator;

	DebugMesh m_mesh[DebugMesh::Count];

	graphics::UniformHandle s_texColor;
	graphics::TextureHandle m_texture;
	graphics::ProgramHandle m_program[Program::Count];
	graphics::UniformHandle u_params;

	graphics::VertexBufferHandle m_vbh;
	graphics::IndexBufferHandle  m_ibh;
};

static DebugDrawShared s_dds;

struct DebugDrawEncoderImpl
{
	DebugDrawEncoderImpl()
		: m_depthTestLess(true)
		, m_state(State::Count)
		, m_defaultEncoder(NULL)
	{
	}

	void init(graphics::Encoder* _encoder)
	{
		m_defaultEncoder = _encoder;
		m_state = State::Count;
	}

	void shutdown()
	{
	}

	void begin(graphics::ViewId _viewId, bool _depthTestLess, graphics::Encoder* _encoder)
	{
		BASE_ASSERT(State::Count == m_state, "");

		m_viewId        = _viewId;
		m_encoder       = _encoder == NULL ? m_defaultEncoder : _encoder;
		m_state         = State::None;
		m_stack         = 0;
		m_depthTestLess = _depthTestLess;

		m_pos       = 0;
		m_indexPos  = 0;
		m_vertexPos = 0;
		m_posQuad   = 0;

		Attrib& attrib = m_attrib[0];
		attrib.m_state = 0
			| GRAPHICS_STATE_WRITE_RGB
			| (m_depthTestLess ? GRAPHICS_STATE_DEPTH_TEST_LESS : GRAPHICS_STATE_DEPTH_TEST_GREATER)
			| GRAPHICS_STATE_CULL_CW
			| GRAPHICS_STATE_WRITE_Z
			;
		attrib.m_scale     = 1.0f;
		attrib.m_spin      = 0.0f;
		attrib.m_offset    = 0.0f;
		attrib.m_abgr      = UINT32_MAX;
		attrib.m_stipple   = false;
		attrib.m_wireframe = false;
		attrib.m_lod       = 0;

		m_mtxStackCurrent = 0;
		m_mtxStack[m_mtxStackCurrent].reset();
	}

	void end()
	{
		BASE_ASSERT(0 == m_stack, "Invalid stack %d.", m_stack);

		flushQuad();
		flush();

		m_encoder = NULL;
		m_state   = State::Count;
	}

	void push()
	{
		BASE_ASSERT(State::Count != m_state, "");
		++m_stack;
		m_attrib[m_stack] = m_attrib[m_stack-1];
	}

	void pop()
	{
		BASE_ASSERT(State::Count != m_state, "");
		const Attrib& curr = m_attrib[m_stack];
		const Attrib& prev = m_attrib[m_stack-1];
		if (curr.m_stipple != prev.m_stipple
		||  curr.m_state   != prev.m_state)
		{
			flush();
		}
		--m_stack;
	}

	void setDepthTestLess(bool _depthTestLess)
	{
		BASE_ASSERT(State::Count != m_state, "");
		if (m_depthTestLess != _depthTestLess)
		{
			m_depthTestLess = _depthTestLess;
			Attrib& attrib = m_attrib[m_stack];
			if (attrib.m_state & GRAPHICS_STATE_DEPTH_TEST_MASK)
			{
				flush();
				attrib.m_state &= ~GRAPHICS_STATE_DEPTH_TEST_MASK;
				attrib.m_state |= _depthTestLess ? GRAPHICS_STATE_DEPTH_TEST_LESS : GRAPHICS_STATE_DEPTH_TEST_GREATER;
			}
		}
	}

	void setTransform(const void* _mtx, uint16_t _num = 1, bool _flush = true)
	{
		BASE_ASSERT(State::Count != m_state, "");
		if (_flush)
		{
			flush();
		}

		MatrixStack& stack = m_mtxStack[m_mtxStackCurrent];

		if (NULL == _mtx)
		{
			stack.reset();
			return;
		}

		graphics::Transform transform;
		stack.mtx  = m_encoder->allocTransform(&transform, _num);
		stack.num  = _num;
		stack.data = transform.data;
		base::memCopy(transform.data, _mtx, _num*64);
	}

	void setTranslate(float _x, float _y, float _z)
	{
		float mtx[16];
		base::mtxTranslate(mtx, _x, _y, _z);
		setTransform(mtx);
	}

	void setTranslate(const float* _pos)
	{
		setTranslate(_pos[0], _pos[1], _pos[2]);
	}

	void pushTransform(const void* _mtx, uint16_t _num, bool _flush = true)
	{
		BASE_ASSERT(m_mtxStackCurrent < BASE_COUNTOF(m_mtxStack), "Out of matrix stack!");
		BASE_ASSERT(State::Count != m_state, "");
		if (_flush)
		{
			flush();
		}

		float* mtx = NULL;

		const MatrixStack& stack = m_mtxStack[m_mtxStackCurrent];

		if (NULL == stack.data)
		{
			mtx = (float*)_mtx;
		}
		else
		{
			mtx = (float*)alloca(_num*64);
			for (uint16_t ii = 0; ii < _num; ++ii)
			{
				const float* mtxTransform = (const float*)_mtx;
				base::mtxMul(&mtx[ii*16], &mtxTransform[ii*16], stack.data);
			}
		}

		m_mtxStackCurrent++;
		setTransform(mtx, _num, _flush);
	}

	void popTransform(bool _flush = true)
	{
		BASE_ASSERT(State::Count != m_state, "");
		if (_flush)
		{
			flush();
		}

		m_mtxStackCurrent--;
	}

	void pushTranslate(float _x, float _y, float _z)
	{
		float mtx[16];
		base::mtxTranslate(mtx, _x, _y, _z);
		pushTransform(mtx, 1);
	}

	void pushTranslate(const base::Vec3& _pos)
	{
		pushTranslate(_pos.x, _pos.y, _pos.z);
	}

	void setState(bool _depthTest, bool _depthWrite, bool _clockwise)
	{
		const uint64_t depthTest = m_depthTestLess
			? GRAPHICS_STATE_DEPTH_TEST_LESS
			: GRAPHICS_STATE_DEPTH_TEST_GREATER
			;

		uint64_t state = m_attrib[m_stack].m_state & ~(0
			| GRAPHICS_STATE_DEPTH_TEST_MASK
			| GRAPHICS_STATE_WRITE_Z
			| GRAPHICS_STATE_CULL_CW
			| GRAPHICS_STATE_CULL_CCW
			);

		state |= _depthTest
			? depthTest
			: 0
			;

		state |= _depthWrite
			? GRAPHICS_STATE_WRITE_Z
			: 0
			;

		state |= _clockwise
			? GRAPHICS_STATE_CULL_CW
			: GRAPHICS_STATE_CULL_CCW
			;

		if (m_attrib[m_stack].m_state != state)
		{
			flush();
		}

		m_attrib[m_stack].m_state = state;
	}

	void setColor(uint32_t _abgr)
	{
		BASE_ASSERT(State::Count != m_state, "");
		m_attrib[m_stack].m_abgr = _abgr;
	}

	void setLod(uint8_t _lod)
	{
		BASE_ASSERT(State::Count != m_state, "");
		m_attrib[m_stack].m_lod = _lod;
	}

	void setWireframe(bool _wireframe)
	{
		BASE_ASSERT(State::Count != m_state, "");
		m_attrib[m_stack].m_wireframe = _wireframe;
	}

	void setStipple(bool _stipple, float _scale = 1.0f, float _offset = 0.0f)
	{
		BASE_ASSERT(State::Count != m_state, "");

		Attrib& attrib = m_attrib[m_stack];

		if (attrib.m_stipple != _stipple)
		{
			flush();
		}

		attrib.m_stipple = _stipple;
		attrib.m_offset  = _offset;
		attrib.m_scale   = _scale;
	}

	void setSpin(float _spin)
	{
		Attrib& attrib = m_attrib[m_stack];
		attrib.m_spin = _spin;
	}

	void moveTo(float _x, float _y, float _z = 0.0f)
	{
		BASE_ASSERT(State::Count != m_state, "");

		softFlush();

		m_state = State::MoveTo;

		DebugVertex& vertex = m_cache[m_pos];
		vertex.m_x = _x;
		vertex.m_y = _y;
		vertex.m_z = _z;

		Attrib& attrib = m_attrib[m_stack];
		vertex.m_abgr = attrib.m_abgr;
		vertex.m_len  = attrib.m_offset;

		m_vertexPos = m_pos;
	}

	void moveTo(const base::Vec3& _pos)
	{
		BASE_ASSERT(State::Count != m_state, "");
		moveTo(_pos.x, _pos.y, _pos.z);
	}

	void moveTo(graphics::Axis::Enum _axis, float _x, float _y)
	{
		moveTo(getPoint(_axis, _x, _y) );
	}

	void lineTo(float _x, float _y, float _z = 0.0f)
	{
		BASE_ASSERT(State::Count != m_state, "");
		if (State::None == m_state)
		{
			moveTo(_x, _y, _z);
			return;
		}

		if (m_pos+2 > uint16_t(BASE_COUNTOF(m_cache) ) )
		{
			uint32_t pos = m_pos;
			uint32_t vertexPos = m_vertexPos;

			flush();

			base::memCopy(&m_cache[0], &m_cache[vertexPos], sizeof(DebugVertex) );
			if (vertexPos == pos)
			{
				m_pos = 1;
			}
			else
			{
				base::memCopy(&m_cache[1], &m_cache[pos - 1], sizeof(DebugVertex) );
				m_pos = 2;
			}

			m_state = State::LineTo;
		}
		else if (State::MoveTo == m_state)
		{
			++m_pos;
			m_state = State::LineTo;
		}

		uint16_t prev = m_pos-1;
		uint16_t curr = m_pos++;
		DebugVertex& vertex = m_cache[curr];
		vertex.m_x = _x;
		vertex.m_y = _y;
		vertex.m_z = _z;

		Attrib& attrib = m_attrib[m_stack];
		vertex.m_abgr = attrib.m_abgr;
		vertex.m_len  = attrib.m_offset;

		float len = base::length(base::sub(base::load<base::Vec3>(&vertex.m_x), base::load<base::Vec3>(&m_cache[prev].m_x) ) ) * attrib.m_scale;
		vertex.m_len = m_cache[prev].m_len + len;

		m_indices[m_indexPos++] = prev;
		m_indices[m_indexPos++] = curr;
	}

	void lineTo(const base::Vec3& _pos)
	{
		BASE_ASSERT(State::Count != m_state, "");
		lineTo(_pos.x, _pos.y, _pos.z);
	}

	void lineTo(graphics::Axis::Enum _axis, float _x, float _y)
	{
		lineTo(getPoint(_axis, _x, _y) );
	}

	void close()
	{
		BASE_ASSERT(State::Count != m_state, "");
		DebugVertex& vertex = m_cache[m_vertexPos];
		lineTo(vertex.m_x, vertex.m_y, vertex.m_z);

		m_state = State::None;
	}

	void draw(const base::Aabb& _aabb)
	{
		const Attrib& attrib = m_attrib[m_stack];
		if (attrib.m_wireframe)
		{
			moveTo(_aabb.min.x, _aabb.min.y, _aabb.min.z);
			lineTo(_aabb.max.x, _aabb.min.y, _aabb.min.z);
			lineTo(_aabb.max.x, _aabb.max.y, _aabb.min.z);
			lineTo(_aabb.min.x, _aabb.max.y, _aabb.min.z);
			close();

			moveTo(_aabb.min.x, _aabb.min.y, _aabb.max.z);
			lineTo(_aabb.max.x, _aabb.min.y, _aabb.max.z);
			lineTo(_aabb.max.x, _aabb.max.y, _aabb.max.z);
			lineTo(_aabb.min.x, _aabb.max.y, _aabb.max.z);
			close();

			moveTo(_aabb.min.x, _aabb.min.y, _aabb.min.z);
			lineTo(_aabb.min.x, _aabb.min.y, _aabb.max.z);

			moveTo(_aabb.max.x, _aabb.min.y, _aabb.min.z);
			lineTo(_aabb.max.x, _aabb.min.y, _aabb.max.z);

			moveTo(_aabb.min.x, _aabb.max.y, _aabb.min.z);
			lineTo(_aabb.min.x, _aabb.max.y, _aabb.max.z);

			moveTo(_aabb.max.x, _aabb.max.y, _aabb.min.z);
			lineTo(_aabb.max.x, _aabb.max.y, _aabb.max.z);
		}
		else
		{
			base::Obb obb;
			toObb(obb, _aabb);
			draw(DebugMesh::Cube, obb.mtx, 1, false);
		}
	}

	void draw(const base::Cylinder& _cylinder, bool _capsule)
	{
		drawCylinder(_cylinder.pos, _cylinder.end, _cylinder.radius, _capsule);
	}

	void draw(const base::Disk& _disk)
	{
		drawCircle(_disk.normal, _disk.center, _disk.radius, 0.0f);
	}

	void draw(const base::Obb& _obb)
	{
		const Attrib& attrib = m_attrib[m_stack];
		if (attrib.m_wireframe)
		{
			pushTransform(_obb.mtx, 1);

			moveTo(-1.0f, -1.0f, -1.0f);
			lineTo( 1.0f, -1.0f, -1.0f);
			lineTo( 1.0f,  1.0f, -1.0f);
			lineTo(-1.0f,  1.0f, -1.0f);
			close();

			moveTo(-1.0f,  1.0f,  1.0f);
			lineTo( 1.0f,  1.0f,  1.0f);
			lineTo( 1.0f, -1.0f,  1.0f);
			lineTo(-1.0f, -1.0f,  1.0f);
			close();

			moveTo( 1.0f, -1.0f, -1.0f);
			lineTo( 1.0f, -1.0f,  1.0f);

			moveTo( 1.0f,  1.0f, -1.0f);
			lineTo( 1.0f,  1.0f,  1.0f);

			moveTo(-1.0f,  1.0f, -1.0f);
			lineTo(-1.0f,  1.0f,  1.0f);

			moveTo(-1.0f, -1.0f, -1.0f);
			lineTo(-1.0f, -1.0f,  1.0f);

			popTransform();
		}
		else
		{
			draw(DebugMesh::Cube, _obb.mtx, 1, false);
		}
	}

	void draw(const base::Sphere& _sphere)
	{
		const Attrib& attrib = m_attrib[m_stack];
		float mtx[16];
		base::mtxSRT(mtx
			, _sphere.radius
			, _sphere.radius
			, _sphere.radius
			, 0.0f
			, 0.0f
			, 0.0f
			, _sphere.center.x
			, _sphere.center.y
			, _sphere.center.z
			);
		uint8_t lod = attrib.m_lod > DebugMesh::SphereMaxLod
			? uint8_t(DebugMesh::SphereMaxLod)
			: attrib.m_lod
			;
		draw(DebugMesh::Enum(DebugMesh::Sphere0 + lod), mtx, 1, attrib.m_wireframe);
	}

	void draw(const base::Triangle& _triangle)
	{
		Attrib& attrib = m_attrib[m_stack];
		if (attrib.m_wireframe)
		{
			moveTo(_triangle.v0);
			lineTo(_triangle.v1);
			lineTo(_triangle.v2);
			close();
		}
		else
		{
			BASE_STATIC_ASSERT(sizeof(DdVertex) == sizeof(base::Vec3), "");

			uint64_t old = attrib.m_state;
			attrib.m_state &= ~GRAPHICS_STATE_CULL_MASK;

			draw(false, 3, reinterpret_cast<const DdVertex*>(&_triangle.v0.x), 0, NULL);

			attrib.m_state = old;
		}
	}

	void setUParams(const Attrib& _attrib, bool _wireframe)
	{
		const float flip = 0 == (_attrib.m_state & GRAPHICS_STATE_CULL_CCW) ? 1.0f : -1.0f;
		const uint8_t alpha = _attrib.m_abgr >> 24;

		float params[4][4] =
		{
			{ // lightDir
				 0.0f * flip,
				-1.0f * flip,
				 0.0f * flip,
				 3.0f, // shininess
			},
			{ // skyColor
				1.0f,
				0.9f,
				0.8f,
				0.0f, // unused
			},
			{ // groundColor.xyz0
				0.2f,
				0.22f,
				0.5f,
				0.0f, // unused
			},
			{ // matColor
				( (_attrib.m_abgr)       & 0xff) / 255.0f,
				( (_attrib.m_abgr >> 8)  & 0xff) / 255.0f,
				( (_attrib.m_abgr >> 16) & 0xff) / 255.0f,
				(alpha) / 255.0f,
			},
		};

		base::store(params[0], base::normalize(base::load<base::Vec3>(params[0]) ) );
		m_encoder->setUniform(s_dds.u_params, params, 4);

		m_encoder->setState(0
			| _attrib.m_state
			| (_wireframe ? GRAPHICS_STATE_PT_LINES | GRAPHICS_STATE_LINEAA | GRAPHICS_STATE_BLEND_ALPHA
			: (alpha < 0xff) ? GRAPHICS_STATE_BLEND_ALPHA : 0)
			);
	}

	void draw(bool _lineList, uint32_t _numVertices, const DdVertex* _vertices, uint32_t _numIndices, const uint16_t* _indices)
	{
		flush();

		if (_numVertices == graphics::getAvailTransientVertexBuffer(_numVertices, DebugMeshVertex::ms_layout) )
		{
			graphics::TransientVertexBuffer tvb;
			graphics::allocTransientVertexBuffer(&tvb, _numVertices, DebugMeshVertex::ms_layout);
			base::memCopy(tvb.data, _vertices, _numVertices * DebugMeshVertex::ms_layout.m_stride);
			m_encoder->setVertexBuffer(0, &tvb);

			const Attrib& attrib = m_attrib[m_stack];
			const bool wireframe = _lineList || attrib.m_wireframe;
			setUParams(attrib, wireframe);

			if (0 < _numIndices)
			{
				uint32_t numIndices = _numIndices;
				graphics::TransientIndexBuffer tib;
				if (!_lineList && wireframe)
				{
					numIndices = graphics::topologyConvert(
						  graphics::TopologyConvert::TriListToLineList
						, NULL
						, 0
						, _indices
						, _numIndices
						, false
						);

					graphics::allocTransientIndexBuffer(&tib, numIndices);
					graphics::topologyConvert(
						  graphics::TopologyConvert::TriListToLineList
						, tib.data
						, numIndices * sizeof(uint16_t)
						, _indices
						, _numIndices
						, false
					);
				}
				else
				{
					graphics::allocTransientIndexBuffer(&tib, numIndices);
					base::memCopy(tib.data, _indices, numIndices * sizeof(uint16_t) );
				}

				m_encoder->setIndexBuffer(&tib);
			}

			m_encoder->setTransform(m_mtxStack[m_mtxStackCurrent].mtx);
			graphics::ProgramHandle program = s_dds.m_program[wireframe
				? Program::FillMesh
				: Program::FillLitMesh
				];
			m_encoder->submit(m_viewId, program);
		}
	}

	void drawFrustum(const float* _viewProj)
	{
		base::Plane planes[6] = { base::InitNone, base::InitNone, base::InitNone, base::InitNone, base::InitNone, base::InitNone };
		buildFrustumPlanes(planes, _viewProj);

		const base::Vec3 points[8] =
		{
			intersectPlanes(planes[0], planes[2], planes[4]),
			intersectPlanes(planes[0], planes[3], planes[4]),
			intersectPlanes(planes[0], planes[3], planes[5]),
			intersectPlanes(planes[0], planes[2], planes[5]),
			intersectPlanes(planes[1], planes[2], planes[4]),
			intersectPlanes(planes[1], planes[3], planes[4]),
			intersectPlanes(planes[1], planes[3], planes[5]),
			intersectPlanes(planes[1], planes[2], planes[5]),
		};

		moveTo(points[0]);
		lineTo(points[1]);
		lineTo(points[2]);
		lineTo(points[3]);
		close();

		moveTo(points[4]);
		lineTo(points[5]);
		lineTo(points[6]);
		lineTo(points[7]);
		close();

		moveTo(points[0]);
		lineTo(points[4]);

		moveTo(points[1]);
		lineTo(points[5]);

		moveTo(points[2]);
		lineTo(points[6]);

		moveTo(points[3]);
		lineTo(points[7]);
	}

	void drawFrustum(const void* _viewProj)
	{
		drawFrustum( (const float*)_viewProj);
	}

	void drawArc(graphics::Axis::Enum _axis, float _x, float _y, float _z, float _radius, float _degrees)
	{
		const Attrib& attrib = m_attrib[m_stack];
		const uint32_t num = getCircleLod(attrib.m_lod);
		const float step = base::kPi * 2.0f / num;

		_degrees = base::wrap(_degrees, 360.0f);

		base::Vec3 pos = getPoint(
			  _axis
			, base::sin(step * 0)*_radius
			, base::cos(step * 0)*_radius
			);

		moveTo({pos.x + _x, pos.y + _y, pos.z + _z});

		uint32_t n = uint32_t(num*_degrees/360.0f);

		for (uint32_t ii = 1; ii < n+1; ++ii)
		{
			pos = getPoint(
				  _axis
				, base::sin(step * ii)*_radius
				, base::cos(step * ii)*_radius
				);
			lineTo({pos.x + _x, pos.y + _y, pos.z + _z});
		}

		moveTo(_x, _y, _z);
		pos = getPoint(
			  _axis
			, base::sin(step * 0)*_radius
			, base::cos(step * 0)*_radius
			);
		lineTo({pos.x + _x, pos.y + _y, pos.z + _z});

		pos = getPoint(
			  _axis
			, base::sin(step * n)*_radius
			, base::cos(step * n)*_radius
			);
		moveTo({pos.x + _x, pos.y + _y, pos.z + _z});
		lineTo(_x, _y, _z);
	}

	void drawCircle(const base::Vec3& _normal, const base::Vec3& _center, float _radius, float _weight)
	{
		const Attrib& attrib = m_attrib[m_stack];
		const uint32_t num = getCircleLod(attrib.m_lod);
		const float step = base::kPi * 2.0f / num;
		_weight = base::clamp(_weight, 0.0f, 2.0f);

		base::Vec3 udir(base::InitNone);
		base::Vec3 vdir(base::InitNone);
		base::calcTangentFrame(udir, vdir, _normal, attrib.m_spin);

		float xy0[2];
		float xy1[2];
		circle(xy0, 0.0f);
		squircle(xy1, 0.0f);

		base::Vec3 pos  = base::mul(udir, base::lerp(xy0[0], xy1[0], _weight)*_radius);
		base::Vec3 tmp0 = base::mul(vdir, base::lerp(xy0[1], xy1[1], _weight)*_radius);
		base::Vec3 tmp1 = base::add(pos,  tmp0);
		base::Vec3 tmp2 = base::add(tmp1, _center);
		moveTo(tmp2);

		for (uint32_t ii = 1; ii < num; ++ii)
		{
			float angle = step * ii;
			circle(xy0, angle);
			squircle(xy1, angle);

			pos  = base::mul(udir, base::lerp(xy0[0], xy1[0], _weight)*_radius);
			tmp0 = base::mul(vdir, base::lerp(xy0[1], xy1[1], _weight)*_radius);
			tmp1 = base::add(pos,  tmp0);
			tmp2 = base::add(tmp1, _center);
			lineTo(tmp2);
		}

		close();
	}

	void drawCircle(graphics::Axis::Enum _axis, float _x, float _y, float _z, float _radius, float _weight)
	{
		const Attrib& attrib = m_attrib[m_stack];
		const uint32_t num = getCircleLod(attrib.m_lod);
		const float step = base::kPi * 2.0f / num;
		_weight = base::clamp(_weight, 0.0f, 2.0f);

		float xy0[2];
		float xy1[2];
		circle(xy0, 0.0f);
		squircle(xy1, 0.0f);

		base::Vec3 pos = getPoint(
			  _axis
			, base::lerp(xy0[0], xy1[0], _weight)*_radius
			, base::lerp(xy0[1], xy1[1], _weight)*_radius
			);

		moveTo({pos.x + _x, pos.y + _y, pos.z + _z});

		for (uint32_t ii = 1; ii < num; ++ii)
		{
			float angle = step * ii;
			circle(xy0, angle);
			squircle(xy1, angle);

			pos = getPoint(
				  _axis
				, base::lerp(xy0[0], xy1[0], _weight)*_radius
				, base::lerp(xy0[1], xy1[1], _weight)*_radius
				);
			lineTo({pos.x + _x, pos.y + _y, pos.z + _z});
		}
		close();
	}

	void drawQuad(const base::Vec3& _normal, const base::Vec3& _center, float _size)
	{
		const Attrib& attrib = m_attrib[m_stack];
		if (attrib.m_wireframe)
		{
			base::Vec3 udir(base::InitNone);
			base::Vec3 vdir(base::InitNone);
			base::calcTangentFrame(udir, vdir, _normal, attrib.m_spin);

			const float halfExtent = _size*0.5f;

			const base::Vec3 umin   = base::mul(udir, -halfExtent);
			const base::Vec3 umax   = base::mul(udir,  halfExtent);
			const base::Vec3 vmin   = base::mul(vdir, -halfExtent);
			const base::Vec3 vmax   = base::mul(vdir,  halfExtent);
			const base::Vec3 center = _center;

			moveTo(base::add(center, base::add(umin, vmin) ) );
			lineTo(base::add(center, base::add(umax, vmin) ) );
			lineTo(base::add(center, base::add(umax, vmax) ) );
			lineTo(base::add(center, base::add(umin, vmax) ) );

			close();
		}
		else
		{
			float mtx[16];
			base::mtxFromNormal(mtx, _normal, _size*0.5f, _center, attrib.m_spin);
			draw(DebugMesh::Quad, mtx, 1, false);
		}
	}

	void drawCone(const base::Vec3& _from, const base::Vec3& _to, float _radius)
	{
		const Attrib& attrib = m_attrib[m_stack];

		const base::Vec3 normal = base::normalize(base::sub(_from, _to) );

		float mtx[2][16];
		base::mtxFromNormal(mtx[0], normal, _radius, _from, attrib.m_spin);

		base::memCopy(mtx[1], mtx[0], 64);
		mtx[1][12] = _to.x;
		mtx[1][13] = _to.y;
		mtx[1][14] = _to.z;

		uint8_t lod = attrib.m_lod > DebugMesh::ConeMaxLod
			? uint8_t(DebugMesh::ConeMaxLod)
			: attrib.m_lod
			;
		draw(DebugMesh::Enum(DebugMesh::Cone0 + lod), mtx[0], 2, attrib.m_wireframe);
	}

	void drawCylinder(const base::Vec3& _from, const base::Vec3& _to, float _radius, bool _capsule)
	{
		const Attrib&  attrib = m_attrib[m_stack];
		const base::Vec3 normal = base::normalize(base::sub(_from, _to) );

		float mtx[2][16];
		base::mtxFromNormal(mtx[0], normal, _radius, _from, attrib.m_spin);

		base::memCopy(mtx[1], mtx[0], 64);
		mtx[1][12] = _to.x;
		mtx[1][13] = _to.y;
		mtx[1][14] = _to.z;

		if (_capsule)
		{
			uint8_t lod = attrib.m_lod > DebugMesh::CapsuleMaxLod
				? uint8_t(DebugMesh::CapsuleMaxLod)
				: attrib.m_lod
				;
			draw(DebugMesh::Enum(DebugMesh::Capsule0 + lod), mtx[0], 2, attrib.m_wireframe);

			base::Sphere sphere;
			sphere.center = _from;
			sphere.radius = _radius;
			draw(sphere);

			sphere.center = _to;
			draw(sphere);
		}
		else
		{
			uint8_t lod = attrib.m_lod > DebugMesh::CylinderMaxLod
				? uint8_t(DebugMesh::CylinderMaxLod)
				: attrib.m_lod
				;
			 draw(DebugMesh::Enum(DebugMesh::Cylinder0 + lod), mtx[0], 2, attrib.m_wireframe);
		}
	}

	void drawAxis(float _x, float _y, float _z, float _len, graphics::Axis::Enum _highlight, float _thickness)
	{
		push();

		if (_thickness > 0.0f)
		{
			const base::Vec3 from = { _x, _y, _z };
			base::Vec3 mid(base::InitNone);
			base::Vec3 to(base::InitNone);

			setColor(graphics::Axis::X == _highlight ? 0xff00ffff : 0xff0000ff);
			mid = { _x + _len - _thickness, _y, _z };
			to  = { _x + _len,              _y, _z };
			drawCylinder(from, mid, _thickness, false);
			drawCone(mid, to, _thickness);

			setColor(graphics::Axis::Y == _highlight ? 0xff00ffff : 0xff00ff00);
			mid = { _x, _y + _len - _thickness, _z };
			to  = { _x, _y + _len,              _z };
			drawCylinder(from, mid, _thickness, false);
			drawCone(mid, to, _thickness);

			setColor(graphics::Axis::Z == _highlight ? 0xff00ffff : 0xffff0000);
			mid = { _x, _y, _z + _len - _thickness };
			to  = { _x, _y, _z + _len              };
			drawCylinder(from, mid, _thickness, false);
			drawCone(mid, to, _thickness);
		}
		else
		{
			setColor(graphics::Axis::X == _highlight ? 0xff00ffff : 0xff0000ff);
			moveTo(_x, _y, _z);
			lineTo(_x + _len, _y, _z);

			setColor(graphics::Axis::Y == _highlight ? 0xff00ffff : 0xff00ff00);
			moveTo(_x, _y, _z);
			lineTo(_x, _y + _len, _z);

			setColor(graphics::Axis::Z == _highlight ? 0xff00ffff : 0xffff0000);
			moveTo(_x, _y, _z);
			lineTo(_x, _y, _z + _len);
		}

		pop();
	}

	void drawGrid(const base::Vec3& _normal, const base::Vec3& _center, uint32_t _size, float _step)
	{
		const Attrib& attrib = m_attrib[m_stack];

		base::Vec3 udir(base::InitNone);
		base::Vec3 vdir(base::InitNone);
		base::calcTangentFrame(udir, vdir, _normal, attrib.m_spin);

		udir = base::mul(udir, _step);
		vdir = base::mul(vdir, _step);

		const uint32_t num = (_size/2)*2+1;
		const float halfExtent = float(_size/2);

		const base::Vec3 umin = base::mul(udir, -halfExtent);
		const base::Vec3 umax = base::mul(udir,  halfExtent);
		const base::Vec3 vmin = base::mul(vdir, -halfExtent);
		const base::Vec3 vmax = base::mul(vdir,  halfExtent);

		base::Vec3 xs = base::add(_center, base::add(umin, vmin) );
		base::Vec3 xe = base::add(_center, base::add(umax, vmin) );
		base::Vec3 ys = base::add(_center, base::add(umin, vmin) );
		base::Vec3 ye = base::add(_center, base::add(umin, vmax) );

		for (uint32_t ii = 0; ii < num; ++ii)
		{
			moveTo(xs);
			lineTo(xe);
			xs = base::add(xs, vdir);
			xe = base::add(xe, vdir);

			moveTo(ys);
			lineTo(ye);
			ys = base::add(ys, udir);
			ye = base::add(ye, udir);
		}
	}

	void drawGrid(graphics::Axis::Enum _axis, const base::Vec3& _center, uint32_t _size, float _step)
	{
		push();
		pushTranslate(_center);

		const uint32_t num = (_size/2)*2-1;
		const float halfExtent = float(_size/2) * _step;

		setColor(0xff606060);
		float yy = -halfExtent + _step;
		for (uint32_t ii = 0; ii < num; ++ii)
		{
			moveTo(_axis, -halfExtent, yy);
			lineTo(_axis,  halfExtent, yy);

			moveTo(_axis, yy, -halfExtent);
			lineTo(_axis, yy,  halfExtent);

			yy += _step;
		}

		setColor(0xff101010);
		moveTo(_axis, -halfExtent, -halfExtent);
		lineTo(_axis, -halfExtent,  halfExtent);
		lineTo(_axis,  halfExtent,  halfExtent);
		lineTo(_axis,  halfExtent, -halfExtent);
		close();

		moveTo(_axis, -halfExtent, 0.0f);
		lineTo(_axis,  halfExtent, 0.0f);

		moveTo(_axis, 0.0f, -halfExtent);
		lineTo(_axis, 0.0f,  halfExtent);

		popTransform();
		pop();
	}

	void drawOrb(float _x, float _y, float _z, float _radius, graphics::Axis::Enum _hightlight)
	{
		push();

		setColor(graphics::Axis::X == _hightlight ? 0xff00ffff : 0xff0000ff);
		drawCircle(graphics::Axis::X, _x, _y, _z, _radius, 0.0f);

		setColor(graphics::Axis::Y == _hightlight ? 0xff00ffff : 0xff00ff00);
		drawCircle(graphics::Axis::Y, _x, _y, _z, _radius, 0.0f);

		setColor(graphics::Axis::Z == _hightlight ? 0xff00ffff : 0xffff0000);
		drawCircle(graphics::Axis::Z, _x, _y, _z, _radius, 0.0f);

		pop();
	}

	void draw(DebugMesh::Enum _mesh, const float* _mtx, uint16_t _num, bool _wireframe)
	{
		pushTransform(_mtx, _num, false /* flush */);

		const DebugMesh& mesh = s_dds.m_mesh[_mesh];

		if (0 != mesh.m_numIndices[_wireframe])
		{
			m_encoder->setIndexBuffer(s_dds.m_ibh
				, mesh.m_startIndex[_wireframe]
				, mesh.m_numIndices[_wireframe]
				);
		}

		const Attrib& attrib = m_attrib[m_stack];
		setUParams(attrib, _wireframe);

		MatrixStack& stack = m_mtxStack[m_mtxStackCurrent];
		m_encoder->setTransform(stack.mtx, stack.num);

		m_encoder->setVertexBuffer(0, s_dds.m_vbh, mesh.m_startVertex, mesh.m_numVertices);
		m_encoder->submit(m_viewId, s_dds.m_program[_wireframe ? Program::Fill : Program::FillLit]);

		popTransform(false /* flush */);
	}

	void softFlush()
	{
		if (m_pos == uint16_t(BASE_COUNTOF(m_cache) ) )
		{
			flush();
		}
	}

	void flush()
	{
		if (0 != m_pos)
		{
			if (checkAvailTransientBuffers(m_pos, DebugVertex::ms_layout, m_indexPos) )
			{
				graphics::TransientVertexBuffer tvb;
				graphics::allocTransientVertexBuffer(&tvb, m_pos, DebugVertex::ms_layout);
				base::memCopy(tvb.data, m_cache, m_pos * DebugVertex::ms_layout.m_stride);

				graphics::TransientIndexBuffer tib;
				graphics::allocTransientIndexBuffer(&tib, m_indexPos);
				base::memCopy(tib.data, m_indices, m_indexPos * sizeof(uint16_t) );

				const Attrib& attrib = m_attrib[m_stack];

				m_encoder->setVertexBuffer(0, &tvb);
				m_encoder->setIndexBuffer(&tib);
				m_encoder->setState(0
					| GRAPHICS_STATE_WRITE_RGB
					| GRAPHICS_STATE_PT_LINES
					| attrib.m_state
					| GRAPHICS_STATE_LINEAA
					| GRAPHICS_STATE_BLEND_ALPHA
					);
				m_encoder->setTransform(m_mtxStack[m_mtxStackCurrent].mtx);
				graphics::ProgramHandle program = s_dds.m_program[attrib.m_stipple ? 1 : 0];
				m_encoder->submit(m_viewId, program);
			}

			m_state     = State::None;
			m_pos       = 0;
			m_indexPos  = 0;
			m_vertexPos = 0;
		}
	}

	inline bool checkAvailTransientBuffers(uint32_t _numVertices, const graphics::VertexLayout& _layout, uint32_t _numIndices)
	{
		return _numVertices == graphics::getAvailTransientVertexBuffer(_numVertices, _layout)
			&& (0 == _numIndices || _numIndices == graphics::getAvailTransientIndexBuffer(_numIndices))
			;
	}

	void flushQuad()
	{
		if (0 != m_posQuad)
		{
			const uint32_t numIndices = m_posQuad/4*6;
			if (checkAvailTransientBuffers(m_posQuad, DebugUvVertex::ms_layout, numIndices) )
			{
				graphics::TransientVertexBuffer tvb;
				graphics::allocTransientVertexBuffer(&tvb, m_posQuad, DebugUvVertex::ms_layout);
				base::memCopy(tvb.data, m_cacheQuad, m_posQuad * DebugUvVertex::ms_layout.m_stride);

				graphics::TransientIndexBuffer tib;
				graphics::allocTransientIndexBuffer(&tib, numIndices);
				uint16_t* indices = (uint16_t*)tib.data;
				for (uint16_t ii = 0, num = m_posQuad/4; ii < num; ++ii)
				{
					uint16_t startVertex = ii*4;
					indices[0] = startVertex+0;
					indices[1] = startVertex+1;
					indices[2] = startVertex+2;
					indices[3] = startVertex+1;
					indices[4] = startVertex+3;
					indices[5] = startVertex+2;
					indices += 6;
				}

				const Attrib& attrib = m_attrib[m_stack];

				m_encoder->setVertexBuffer(0, &tvb);
				m_encoder->setIndexBuffer(&tib);
				m_encoder->setState(0
					| (attrib.m_state & ~GRAPHICS_STATE_CULL_MASK)
					);
				m_encoder->setTransform(m_mtxStack[m_mtxStackCurrent].mtx);
				m_encoder->setTexture(0, s_dds.s_texColor, s_dds.m_texture);
				m_encoder->submit(m_viewId, s_dds.m_program[Program::FillTexture]);
			}

			m_posQuad = 0;
		}
	}

	struct State
	{
		enum Enum
		{
			None,
			MoveTo,
			LineTo,

			Count
		};
	};

	static const uint32_t kCacheSize = 1024;
	static const uint32_t kStackSize = 16;
	static const uint32_t kCacheQuadSize = 1024;
	BASE_STATIC_ASSERT(kCacheSize >= 3, "Cache must be at least 3 elements.");

	DebugVertex   m_cache[kCacheSize+1];
	DebugUvVertex m_cacheQuad[kCacheQuadSize];
	uint16_t m_indices[kCacheSize*2];
	uint16_t m_pos;
	uint16_t m_posQuad;
	uint16_t m_indexPos;
	uint16_t m_vertexPos;
	uint32_t m_mtxStackCurrent;

	struct MatrixStack
	{
		void reset()
		{
			mtx  = 0;
			num  = 1;
			data = NULL;
		}

		uint32_t mtx;
		uint16_t num;
		float*   data;
	};

	MatrixStack m_mtxStack[32];

	graphics::ViewId m_viewId;
	uint8_t m_stack;
	bool    m_depthTestLess;

	Attrib m_attrib[kStackSize];

	State::Enum m_state;

	graphics::Encoder* m_encoder;
	graphics::Encoder* m_defaultEncoder;
};

static DebugDrawEncoderImpl s_dde;
BASE_STATIC_ASSERT(sizeof(DebugDrawEncoderImpl) <= sizeof(DebugDrawEncoder), "Size must match");

void ddInit(base::AllocatorI* _allocator)
{
	s_dds.init(_allocator);
	s_dde.init(graphics::begin() );
}

void ddShutdown()
{
	s_dde.shutdown();
	s_dds.shutdown();
}

#define DEBUG_DRAW_ENCODER(_func) reinterpret_cast<DebugDrawEncoderImpl*>(this)->_func

DebugDrawEncoder::DebugDrawEncoder()
{
	DEBUG_DRAW_ENCODER(init(s_dde.m_defaultEncoder) );
}

DebugDrawEncoder::~DebugDrawEncoder()
{
	DEBUG_DRAW_ENCODER(shutdown() );
}

void DebugDrawEncoder::begin(uint16_t _viewId, bool _depthTestLess, graphics::Encoder* _encoder)
{
	DEBUG_DRAW_ENCODER(begin(_viewId, _depthTestLess, _encoder) );
}

void DebugDrawEncoder::end()
{
	DEBUG_DRAW_ENCODER(end() );
}

void DebugDrawEncoder::push()
{
	DEBUG_DRAW_ENCODER(push() );
}

void DebugDrawEncoder::pop()
{
	DEBUG_DRAW_ENCODER(pop() );
}

void DebugDrawEncoder::setDepthTestLess(bool _depthTestLess)
{
	DEBUG_DRAW_ENCODER(setDepthTestLess(_depthTestLess) );
}

void DebugDrawEncoder::setState(bool _depthTest, bool _depthWrite, bool _clockwise)
{
	DEBUG_DRAW_ENCODER(setState(_depthTest, _depthWrite, _clockwise) );
}

void DebugDrawEncoder::setColor(uint32_t _abgr)
{
	DEBUG_DRAW_ENCODER(setColor(_abgr) );
}

void DebugDrawEncoder::setLod(uint8_t _lod)
{
	DEBUG_DRAW_ENCODER(setLod(_lod) );
}

void DebugDrawEncoder::setWireframe(bool _wireframe)
{
	DEBUG_DRAW_ENCODER(setWireframe(_wireframe) );
}

void DebugDrawEncoder::setStipple(bool _stipple, float _scale, float _offset)
{
	DEBUG_DRAW_ENCODER(setStipple(_stipple, _scale, _offset) );
}

void DebugDrawEncoder::setSpin(float _spin)
{
	DEBUG_DRAW_ENCODER(setSpin(_spin) );
}

void DebugDrawEncoder::setTransform(const void* _mtx)
{
	DEBUG_DRAW_ENCODER(setTransform(_mtx) );
}

void DebugDrawEncoder::setTranslate(float _x, float _y, float _z)
{
	DEBUG_DRAW_ENCODER(setTranslate(_x, _y, _z) );
}

void DebugDrawEncoder::pushTransform(const void* _mtx)
{
	DEBUG_DRAW_ENCODER(pushTransform(_mtx, 1) );
}

void DebugDrawEncoder::popTransform()
{
	DEBUG_DRAW_ENCODER(popTransform() );
}

void DebugDrawEncoder::moveTo(float _x, float _y, float _z)
{
	DEBUG_DRAW_ENCODER(moveTo(_x, _y, _z) );
}

void DebugDrawEncoder::moveTo(const base::Vec3& _pos)
{
	DEBUG_DRAW_ENCODER(moveTo(_pos) );
}

void DebugDrawEncoder::lineTo(float _x, float _y, float _z)
{
	DEBUG_DRAW_ENCODER(lineTo(_x, _y, _z) );
}

void DebugDrawEncoder::lineTo(const base::Vec3& _pos)
{
	DEBUG_DRAW_ENCODER(lineTo(_pos) );
}

void DebugDrawEncoder::close()
{
	DEBUG_DRAW_ENCODER(close() );
}

void DebugDrawEncoder::draw(const base::Aabb& _aabb)
{
	DEBUG_DRAW_ENCODER(draw(_aabb) );
}

void DebugDrawEncoder::draw(const base::Cylinder& _cylinder)
{
	DEBUG_DRAW_ENCODER(draw(_cylinder, false) );
}

void DebugDrawEncoder::draw(const base::Capsule& _capsule)
{
	DEBUG_DRAW_ENCODER(draw(*( (const base::Cylinder*)&_capsule), true) );
}

void DebugDrawEncoder::draw(const base::Disk& _disk)
{
	DEBUG_DRAW_ENCODER(draw(_disk) );
}

void DebugDrawEncoder::draw(const base::Obb& _obb)
{
	DEBUG_DRAW_ENCODER(draw(_obb) );
}

void DebugDrawEncoder::draw(const base::Sphere& _sphere)
{
	DEBUG_DRAW_ENCODER(draw(_sphere) );
}

void DebugDrawEncoder::draw(const base::Triangle& _triangle)
{
	DEBUG_DRAW_ENCODER(draw(_triangle) );
}

void DebugDrawEncoder::draw(const base::Cone& _cone)
{
	DEBUG_DRAW_ENCODER(drawCone(_cone.pos, _cone.end, _cone.radius) );
}

void DebugDrawEncoder::drawLineList(uint32_t _numVertices, const DdVertex* _vertices, uint32_t _numIndices, const uint16_t* _indices)
{
	DEBUG_DRAW_ENCODER(draw(true, _numVertices, _vertices, _numIndices, _indices) );
}

void DebugDrawEncoder::drawTriList(uint32_t _numVertices, const DdVertex* _vertices, uint32_t _numIndices, const uint16_t* _indices)
{
	DEBUG_DRAW_ENCODER(draw(false, _numVertices, _vertices, _numIndices, _indices) );
}

void DebugDrawEncoder::drawFrustum(const void* _viewProj)
{
	DEBUG_DRAW_ENCODER(drawFrustum(_viewProj) );
}

void DebugDrawEncoder::drawArc(graphics::Axis::Enum _axis, float _x, float _y, float _z, float _radius, float _degrees)
{
	DEBUG_DRAW_ENCODER(drawArc(_axis, _x, _y, _z, _radius, _degrees) );
}

void DebugDrawEncoder::drawCircle(const base::Vec3& _normal, const base::Vec3& _center, float _radius, float _weight)
{
	DEBUG_DRAW_ENCODER(drawCircle(_normal, _center, _radius, _weight) );
}

void DebugDrawEncoder::drawCircle(graphics::Axis::Enum _axis, float _x, float _y, float _z, float _radius, float _weight)
{
	DEBUG_DRAW_ENCODER(drawCircle(_axis, _x, _y, _z, _radius, _weight) );
}

void DebugDrawEncoder::drawQuad(const base::Vec3& _normal, const base::Vec3& _center, float _size)
{
	DEBUG_DRAW_ENCODER(drawQuad(_normal, _center, _size) );
}

void DebugDrawEncoder::drawCone(const base::Vec3& _from, const base::Vec3& _to, float _radius)
{
	DEBUG_DRAW_ENCODER(drawCone(_from, _to, _radius) );
}

void DebugDrawEncoder::drawCylinder(const base::Vec3& _from, const base::Vec3& _to, float _radius)
{
	DEBUG_DRAW_ENCODER(drawCylinder(_from, _to, _radius, false) );
}

void DebugDrawEncoder::drawCapsule(const base::Vec3& _from, const base::Vec3& _to, float _radius)
{
	DEBUG_DRAW_ENCODER(drawCylinder(_from, _to, _radius, true) );
}

void DebugDrawEncoder::drawAxis(float _x, float _y, float _z, float _len, graphics::Axis::Enum _highlight, float _thickness)
{
	DEBUG_DRAW_ENCODER(drawAxis(_x, _y, _z, _len, _highlight, _thickness) );
}

void DebugDrawEncoder::drawGrid(const base::Vec3& _normal, const base::Vec3& _center, uint32_t _size, float _step)
{
	DEBUG_DRAW_ENCODER(drawGrid(_normal, _center, _size, _step) );
}

void DebugDrawEncoder::drawGrid(graphics::Axis::Enum _axis, const base::Vec3& _center, uint32_t _size, float _step)
{
	DEBUG_DRAW_ENCODER(drawGrid(_axis, _center, _size, _step) );
}

void DebugDrawEncoder::drawOrb(float _x, float _y, float _z, float _radius, graphics::Axis::Enum _highlight)
{
	DEBUG_DRAW_ENCODER(drawOrb(_x, _y, _z, _radius, _highlight) );
}

DebugDrawEncoderScopePush::DebugDrawEncoderScopePush(DebugDrawEncoder& _dde)
	: m_dde(_dde)
{
	m_dde.push();
}

DebugDrawEncoderScopePush::~DebugDrawEncoderScopePush()
{
	m_dde.pop();
}
