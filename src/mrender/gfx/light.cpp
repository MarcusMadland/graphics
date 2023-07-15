#include "mrender/gfx/light.hpp"

namespace mrender {

LightImplementation::LightImplementation(const LightSettings& settings)
	: mSettings(settings)
{
}

void LightImplementation::setSettings(const LightSettings& settings)
{
	mSettings = settings;
}

}	// namespace mrender