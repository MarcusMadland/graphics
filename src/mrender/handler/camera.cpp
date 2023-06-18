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
    float cam_rotation[16];
    bx::mtxRotateXYZ(cam_rotation, mSettings.rotation[0], mSettings.rotation[1], mSettings.rotation[2]);

    float cam_translation[16];
    bx::mtxTranslate(cam_translation, mSettings.postion[0], mSettings.postion[1], mSettings.postion[2]);

    float cam_transform[16];
    bx::mtxMul(cam_transform, cam_translation, cam_rotation);

    bx::mtxInverse(mView, cam_transform);

    bx::mtxProj(
        mProj, mSettings.fov, mSettings.width / mSettings.height, mSettings.clipNear,
        mSettings.clipFar, bgfx::getCaps()->homogeneousDepth);
}

void CameraImplementation::setSettings(const CameraSettings& settings)
{
	mSettings = settings;
	recalculate();
}

}	// namespace mrender
