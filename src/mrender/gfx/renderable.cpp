#include "mrender/gfx/renderable.hpp"

#include <bx/math.h>

namespace mrender {

RenderableImplementation::RenderableImplementation(std::shared_ptr<Geometry> geometry, const std::string_view& shader)
	: mGeometry(std::move(geometry)), mShader(shader)
{
	bx::mtxIdentity(mTransform);
}

}	// namespace mrender