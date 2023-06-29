#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

	class Empty : public RenderSystem
	{
	public:
		Empty();
		~Empty();

		bool init(RenderContext& context) override;
		void render(RenderContext& context) override;

		std::unordered_map<std::string, std::shared_ptr<Texture>> getBuffers(RenderContext& context) override;

	private:
		std::shared_ptr<RenderState> mState;
	};
}	// namespace mrender