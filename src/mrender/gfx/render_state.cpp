#include "mrender/gfx/render_state.hpp"
#include "mrender/gfx/render_context.hpp"

namespace mrender {

	RenderStateImplementation::RenderStateImplementation(GfxContext* context, std::string_view name, uint64_t flags, RenderOrder order)
		: mFlags(flags), mOrder(order)
	{
		auto contextImpl = static_cast<GfxContextImplementation*>(context);
		mId = contextImpl->getRenderStateCount() + 1;
		contextImpl->setRenderStateCount(mId);
		bgfx::setViewName(mId, name.data());
	}

}	// namespace mrender