#include "mrender/gfx/renderable.hpp"

#include <bx/math.h>

namespace mrender {

RenderableImplementation::RenderableImplementation(GeometryHandle geometry, MaterialHandle material)
	: mGeometry(geometry), mMaterial(material)
{
	bx::mtxIdentity(mTransform);
}

}	// namespace mrender