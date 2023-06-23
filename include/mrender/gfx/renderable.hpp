#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class RenderableImplementation final : public Renderable
{
public:
	RenderableImplementation(std::shared_ptr<Geometry> geometry, const std::string_view& shader);

	virtual void setTransform(float matrix[16]) override { for (int i = 0; i < 16; i++)  mTransform[i] = matrix[i]; }
	[[nodiscard]] virtual float* getTransform() { return mTransform; }

	[[nodiscard]] virtual std::shared_ptr<Geometry> getGeometry() { return mGeometry; }
	[[nodiscard]] virtual const std::string_view getShader() const { return mShader; }

private:
	float mTransform[16];
	std::shared_ptr<Geometry> mGeometry = nullptr;
	std::string_view mShader;
};

}