/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#ifndef DEBUGDRAW_H_HEADER_GUARD
#define DEBUGDRAW_H_HEADER_GUARD

#include <base/allocator.h>
#include <base/bounds.h>
#include <graphics/graphics.h>

struct DdVertex
{
	float x, y, z;
};

///
void ddInit(base::AllocatorI* _allocator = NULL);

///
void ddShutdown();

///
struct DebugDrawEncoder
{
	///
	DebugDrawEncoder();

	///
	~DebugDrawEncoder();

	///
	void begin(uint16_t _viewId, bool _depthTestLess = true, graphics::Encoder* _encoder = NULL);

	///
	void end();

	///
	void push();

	///
	void pop();

	///
	void setDepthTestLess(bool _depthTestLess);

	///
	void setState(bool _depthTest, bool _depthWrite, bool _clockwise);

	///
	void setColor(uint32_t _abgr);

	///
	void setLod(uint8_t _lod);

	///
	void setWireframe(bool _wireframe);

	///
	void setStipple(bool _stipple, float _scale = 1.0f, float _offset = 0.0f);

	///
	void setSpin(float _spin);

	///
	void setTransform(const void* _mtx);

	///
	void setTranslate(float _x, float _y, float _z);

	///
	void pushTransform(const void* _mtx);

	///
	void popTransform();

	///
	void moveTo(float _x, float _y, float _z = 0.0f);

	///
	void moveTo(const base::Vec3& _pos);

	///
	void lineTo(float _x, float _y, float _z = 0.0f);

	///
	void lineTo(const base::Vec3& _pos);

	///
	void close();

	///
	void draw(const base::Aabb& _aabb);

	///
	void draw(const base::Cylinder& _cylinder);

	///
	void draw(const base::Capsule& _capsule);

	///
	void draw(const base::Disk& _disk);

	///
	void draw(const base::Obb& _obb);

	///
	void draw(const base::Sphere& _sphere);

	///
	void draw(const base::Triangle& _triangle);

	///
	void draw(const base::Cone& _cone);

	///
	void drawLineList(uint32_t _numVertices, const DdVertex* _vertices, uint32_t _numIndices = 0, const uint16_t* _indices = NULL);

	///
	void drawTriList(uint32_t _numVertices, const DdVertex* _vertices, uint32_t _numIndices = 0, const uint16_t* _indices = NULL);

	///
	void drawFrustum(const void* _viewProj);

	///
	void drawArc(graphics::Axis::Enum _axis, float _x, float _y, float _z, float _radius, float _degrees);

	///
	void drawCircle(const base::Vec3& _normal, const base::Vec3& _center, float _radius, float _weight = 0.0f);

	///
	void drawCircle(graphics::Axis::Enum _axis, float _x, float _y, float _z, float _radius, float _weight = 0.0f);

	///
	void drawQuad(const base::Vec3& _normal, const base::Vec3& _center, float _size);

	///
	void drawCone(const base::Vec3& _from, const base::Vec3& _to, float _radius);

	///
	void drawCylinder(const base::Vec3& _from, const base::Vec3& _to, float _radius);

	///
	void drawCapsule(const base::Vec3& _from, const base::Vec3& _to, float _radius);

	///
	void drawAxis(float _x, float _y, float _z, float _len = 1.0f, graphics::Axis::Enum _highlight = graphics::Axis::Count, float _thickness = 0.0f);

	///
	void drawGrid(const base::Vec3& _normal, const base::Vec3& _center, uint32_t _size = 20, float _step = 1.0f);

	///
	void drawGrid(graphics::Axis::Enum _axis, const base::Vec3& _center, uint32_t _size = 20, float _step = 1.0f);

	///
	void drawOrb(float _x, float _y, float _z, float _radius, graphics::Axis::Enum _highlight = graphics::Axis::Count);

	BASE_ALIGN_DECL_CACHE_LINE(uint8_t) m_internal[50<<10];
};

///
class DebugDrawEncoderScopePush
{
public:
	///
	DebugDrawEncoderScopePush(DebugDrawEncoder& _dde);

	///
	~DebugDrawEncoderScopePush();

private:
	DebugDrawEncoder& m_dde;
};

#endif // DEBUGDRAW_H_HEADER_GUARD
