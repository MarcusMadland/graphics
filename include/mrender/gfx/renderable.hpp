#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class RenderableImplementation final : public Renderable
{
public:
	RenderableImplementation(std::shared_ptr<Geometry> geometry, std::shared_ptr<Material> material);

	virtual void setTransform(float matrix[16]) override { for (int i = 0; i < 16; i++)  mTransform[i] = matrix[i]; }
	[[nodiscard]] virtual float* getTransform() override { return mTransform; }
	[[nodiscard]] virtual std::shared_ptr<Geometry> getGeometry() override { return mGeometry; }
	[[nodiscard]] virtual std::shared_ptr<Material> getMaterial() override { return mMaterial; }

private:
	float mTransform[16];
	std::shared_ptr<Geometry> mGeometry;
	std::shared_ptr<Material> mMaterial;
};

}