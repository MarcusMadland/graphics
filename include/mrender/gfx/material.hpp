#pragma once

#include "mrender/mrender.hpp"

#include "mrender/gfx/render_context.hpp"

namespace mrender {

class MaterialImplementation : public Material
{
public:
	MaterialImplementation(GfxContext* context, ShaderHandle shader);

	virtual void setParameter(std::string name, UniformData::UniformType type, std::shared_ptr<void> data) ;
	[[nodiscard]] virtual const ParameterList& getParameters()  { return mUniformData; };
	[[nodiscard]] virtual const ShaderHandle getShader() const  { return mShader; }

private:
	GfxContext* mContext;
	ShaderHandle mShader;
	ParameterList mUniformData;
};

}	// namespace mrender