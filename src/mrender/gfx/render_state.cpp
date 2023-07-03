#include "mrender/gfx/render_state.hpp"
#include "mrender/gfx/render_context.hpp"

namespace mrender {

	RenderStateImplementation::RenderStateImplementation(GfxContext* context, uint64_t flags)
		: mFlags(flags)
	{
		auto contextImpl = static_cast<GfxContextImplementation*>(context);
		mId = contextImpl->getRenderStateCount() + 1;
		contextImpl->setRenderStateCount(mId);
	}

}	// namespace mrender