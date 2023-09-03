#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class RenderableImplementation final : public Renderable
{
public:
	RenderableImplementation(GeometryHandle geometry, MaterialHandle material);

	
	virtual void setTransform(float matrix[16]) { for (int i = 0; i < 16; i++)  mTransform[i] = matrix[i]; }
	 virtual float* getTransform() { return mTransform; }

	 virtual GeometryHandle getGeometry() { return mGeometry; }

	virtual void setMaterial(MaterialHandle material) { mMaterial = material; }
	 virtual MaterialHandle getMaterial() { return mMaterial; }

private:
	float mTransform[16];
	GeometryHandle mGeometry;
	MaterialHandle mMaterial;
};

}	// namespace mrender
