#include "mrender/systems/empty/empty.hpp"

#include <bgfx/bgfx.h>

namespace mrender {

Empty::Empty(GfxContext* context)
	: RenderSystem("Empty")
{
}

Empty::~Empty()
{
}

bool Empty::init(GfxContext* context)
{
	return true;
}

void Empty::render(GfxContext* context)
{
}

}	// namespace mrender