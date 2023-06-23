#include "mrender/handler/render_state.hpp"
#include "mrender/handler/render_context.hpp"

namespace mrender {

	RenderStateImplementation::RenderStateImplementation(RenderContext& context, uint64_t flags)
		: mFlags(flags)
	{
		auto contextImpl = static_cast<RenderContextImplementation&>(context);
		mId = context.getRenderStateCount() + 1;
		context.setRenderStateCount(mId);
	}

}	// namespace mrender