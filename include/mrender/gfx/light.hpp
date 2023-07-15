#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class LightImplementation : public Light
{
	friend class GfxContextImplementation;

public:
	LightImplementation(const LightSettings& settings);

	void setSettings(const LightSettings& settings);
	[[nodiscard]] const LightSettings& getSettings() { return mSettings; };

private: // @todo I want this public like a struct but its bad design. Figure it out
	LightSettings mSettings;
};

}	// namespace mrender