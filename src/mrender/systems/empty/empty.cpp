#include "mrender/systems/empty/empty.hpp"

#include <bgfx/bgfx.h> // @todo Make a wrapper around bgfx tags for tags we want to support

namespace mrender {

	Empty::Empty()
		: RenderSystem("Empty")
	{
	}

	Empty::~Empty()
	{
	}

	bool Empty::init(RenderContext& context)
	{
		//mState = context.createRenderState("Empty", BGFX_STATE_DEFAULT);

		return true;
	}

	void Empty::render(RenderContext& context)
	{
		PROFILE_SCOPE(mName);

		//context.setRenderState(mState);
		//context.clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
	}

	std::unordered_map<std::string, std::shared_ptr<Texture>> Empty::getBuffers(RenderContext& context)
	{
		std::unordered_map<std::string, std::shared_ptr<Texture>> buffers;
		return buffers;
	}

}	// namespace mrender