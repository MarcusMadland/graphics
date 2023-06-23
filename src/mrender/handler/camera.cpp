#include "mrender/handler/camera.hpp"

#include <bx/math.h>
#include <bgfx/bgfx.h>

namespace mrender {

CameraImplementation::CameraImplementation(const CameraSettings& settings)
	: Camera(settings)
{
	recalculate();
}

void CameraImplementation::recalculate()
{
    // View matrix
    bx::mtxLookAt(&mView[0],
        { mSettings.mPosition[0], mSettings.mPosition[1], mSettings.mPosition[2] },
        { mSettings.mLookAt[0], mSettings.mLookAt[1], mSettings.mLookAt[2] });

    // Projection matrix
    switch (mSettings.mProjectionType)
    {
    case mrender::ProjectionType::Perspective:
    {
        bx::mtxProj(
            mProj, mSettings.mFov, mSettings.mWidth / mSettings.mHeight, mSettings.mClipNear,
            mSettings.mClipFar, bgfx::getCaps()->homogeneousDepth);
        break;
    }
    case mrender::ProjectionType::Orthographic:
    {
        const float x = mSettings.mWidth / 2.0f;
        const float y = mSettings.mHeight / 2.0f;
        bx::mtxOrtho(mProj, -x, x, -y, y, mSettings.mClipNear,
            mSettings.mClipFar, 0.0f, bgfx::getCaps()->homogeneousDepth);
        break;
    }
    default: { break; }
    }
    
}

void CameraImplementation::setSettings(const CameraSettings& settings)
{
	mSettings = settings;
	recalculate();
}

}	// namespace mrender
