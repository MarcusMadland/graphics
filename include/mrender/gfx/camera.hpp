#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class CameraImplementation : public Camera
{
	friend class RenderContextImplementation;

public:
	CameraImplementation(const CameraSettings& settings);

	virtual void recalculate() override;

	virtual void setSettings(const CameraSettings& settings) override;
	virtual [[nodiscard]] const CameraSettings getSettings() const override { return mSettings; }

	[[nodiscard]] float* getViewMatrix() { return mView; }
	[[nodiscard]] float* getProjMatrix() { return mProj; }

private:
	CameraSettings mSettings;
	float mView[16];
	float mProj[16];
};

}	// namespace mrender