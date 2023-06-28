#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

	class RenderStateImplementation : public RenderState
	{
		friend class RenderContextImplementation;

	public:
		RenderStateImplementation(RenderContext& context, std::string name, uint64_t flags);

	private:
		uint16_t mId;
		uint64_t mFlags;
	};

}	// namespace mrender