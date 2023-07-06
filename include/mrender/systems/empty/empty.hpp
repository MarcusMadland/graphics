#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class Empty : public RenderSystem
{
public:
	Empty();
	~Empty();

	bool init(GfxContext* context) override;
	void render(GfxContext* context) override;

	BufferList getBuffers(GfxContext* context) override;
	UniformDataList getUniformData(GfxContext* context) override;

private:
	RenderStateRef mState;
};

}	// namespace mrender