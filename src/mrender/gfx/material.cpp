#include "mrender/gfx/material.hpp"
#include "mrender/gfx/shader.hpp"

namespace mrender {

MaterialImplementation::MaterialImplementation(RenderContext& context, const std::string& shaderName)
	: mContext(context), mShaderName(shaderName)
{
	mUniformData.clear();
}

void MaterialImplementation::setUniform(std::string name, UniformType type, std::shared_ptr<void> data)
{
	if (data == nullptr) printf("Setting uniform %s with invalid data\n", name.data());

	auto shaderImpl = std::static_pointer_cast<ShaderImplementation>(mContext.getShaders().at(mShaderName));
	if (shaderImpl->mUniformHandles.count(name) > 0)
	{
		mUniformData[name] = { type, std::move(data) };
	}
	else
	{
		printf("Failed to set uniform in material. Shader uniform do not exist or is not in use inside shader\n");
	}
}

}	// namespace mrender