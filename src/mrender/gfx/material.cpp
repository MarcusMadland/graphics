#include "mrender/gfx/material.hpp"
#include "mrender/gfx/shader.hpp"

namespace mrender {

MaterialImplementation::MaterialImplementation(GfxContext* context, ShaderHandle shader)
	: mContext(context), mShader(shader)
{
}

void MaterialImplementation::setParameter(std::string name, UniformData::UniformType type, std::shared_ptr<void> data)
{
	if (data == nullptr) printf("Setting uniform %s with invalid data\n", name.data());

	auto contextImpl = static_cast<GfxContextImplementation*>(mContext);
	auto shaderImpl = STATIC_IMPL_CAST(Shader, contextImpl->mShaders.at(mShader.idx));
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