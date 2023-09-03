#pragma once

#include "mrender/mrender.hpp"

#include "mrender/gfx/render_context.hpp"

namespace mrender {

class MaterialImplementation : public Material
{
public:
	MaterialImplementation(GfxContext* context, ShaderHandle shader);

	void setUniformData(std::string name, UniformData::UniformType type, void* data);
	void setTextureData(std::string name, TextureHandle data);
	 const UniformDataList& getUniformDataList()  { return mUniformData; };
	 const TextureDataList& getTextureDataList() { return mTextureData; }
	 const ShaderHandle getShader() { return mShader; }

private:
	GfxContext* mContext;
	ShaderHandle mShader;
	UniformDataList mUniformData;
	TextureDataList mTextureData;
};

}	// namespace mrender
