#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class MaterialImplementation : public Material
{
public:
	MaterialImplementation(RenderContext& context, const std::string& shaderName);

	virtual void setUniform(std::string name, UniformType type, std::shared_ptr<void> data) override;
	[[nodiscard]] virtual const std::unordered_map<std::string, UniformData>& getUniformData() override { return mUniformData; };
	[[nodiscard]] virtual const std::string getShaderName() const override { return mShaderName; }

private:
	RenderContext& mContext;
	std::string mShaderName;
	std::unordered_map<std::string, UniformData> mUniformData;
};

}	// namespace mrender