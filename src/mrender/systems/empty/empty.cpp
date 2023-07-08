#include "mrender/systems/empty/empty.hpp"

#include <bgfx/bgfx.h>

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
	mEmptyRenderState = context->createRenderState("Debug", 0
		| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_MSAA);
	
	mEmptyFramebuffer = context->createFramebuffer(mBufferList);

	return true;
}

void Empty::render(GfxContext* context)
{
	PROFILE_SCOPE(mName);

	context->setActiveRenderState(mEmptyRenderState);
	context->setActiveFramebuffer(mEmptyFramebuffer);
	context->clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
}

BufferList Empty::getBuffers(GfxContext* context)
{
	mBufferList.emplace("DebugDrawColor", context->createTexture(TextureFormat::BGRA8, BGFX_TEXTURE_RT));
	mBufferList.emplace("DebugDrawDepth", context->createTexture(TextureFormat::D32F, BGFX_TEXTURE_RT));
	return mBufferList;
}

UniformDataList Empty::getUniformData(GfxContext* context)
{
	return UniformDataList();
}

}	// namespace mrender