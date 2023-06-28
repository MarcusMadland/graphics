#include "mrender/gfx/render_state.hpp"
#include "mrender/gfx/render_context.hpp"

namespace mrender {

	RenderStateImplementation::RenderStateImplementation(RenderContext& context, std::string name, uint64_t flags)
		: mFlags(flags)
	{
		auto contextImpl = static_cast<RenderContextImplementation&>(context);
		mId = context.getRenderStateCount() + 1;
		context.setRenderStateCount(mId);
		bgfx::setViewName(mId, name.data());
	}

}	// namespace mrender