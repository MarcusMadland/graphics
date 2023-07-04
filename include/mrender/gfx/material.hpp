#pragma once

#include "mrender/mrender.hpp"

#include "mrender/gfx/render_context.hpp"

namespace mrender {

class MaterialImplementation : public Material
{
public:
	MaterialImplementation(GfxContext* context, ShaderHandle shader);

	void setUniformData(std::string name, UniformData::UniformType type, std::shared_ptr<void> data);
	void setTextureData(std::string name, TextureHandle data);
	[[nodiscard]] const UniformDataList& getUniformDataList()  { return mUniformData; };
	[[nodiscard]] const TextureDataList& getTextureDataList() { return mTextureData; }
	[[nodiscard]] const ShaderHandle getShader() { return mShader; }

private:
	GfxContext* mContext;
	ShaderHandle mShader;
	UniformDataList mUniformData;
	TextureDataList mTextureData;
};

}	// namespace mrender