#pragma once

#include "mrender/mrender.hpp"

namespace mrender {

class CameraImplementation : public Camera
{
	friend class GfxContextImplementation;

public:
	CameraImplementation(const CameraSettings& settings);

	virtual void recalculate() ;

	virtual void setSettings(const CameraSettings& settings) ;
	virtual [[nodiscard]] const CameraSettings getSettings() { return mSettings; }

	[[nodiscard]] float* getViewMatrix() { return mView; }
	[[nodiscard]] float* getProjMatrix() { return mProj; }
	[[nodiscard]] float* getViewProjMatrix() { return mViewProj; }

private:
	CameraSettings mSettings;
	float mView[16];
	float mProj[16];
	float mViewProj[16];
};

}	// namespace mrender