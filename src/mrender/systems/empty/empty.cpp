#include "mrender/systems/empty/empty.hpp"

namespace mrender {

Empty::Empty()
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
	PROFILE_SCOPE(mName);

}

BufferList Empty::getBuffers(GfxContext* context)
{
	BufferList buffers;
	return buffers;
}

UniformDataList Empty::getUniformData(GfxContext* context)
{
	return UniformDataList();
}

}	// namespace mrender