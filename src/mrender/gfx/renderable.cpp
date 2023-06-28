#include "mrender/gfx/renderable.hpp"

#include <bx/math.h>

namespace mrender {

RenderableImplementation::RenderableImplementation(std::shared_ptr<Geometry> geometry, std::shared_ptr<Material> material)
	: mGeometry(std::move(geometry)), mMaterial(std::move(material))
{
	bx::mtxIdentity(mTransform);
}

}	// namespace mrender