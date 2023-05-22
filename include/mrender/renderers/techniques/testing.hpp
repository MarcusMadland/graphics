#pragma once

#include "mrender/renderers/renderer.hpp"

namespace mrender {

struct TestingRendererSettings : public TechniqueSettings
{

};

class TestingTechnique : public Renderer
{
public:
	virtual void initialize(RenderContext& context) override;
	virtual void render() override;
};

}   // namespace mrender
