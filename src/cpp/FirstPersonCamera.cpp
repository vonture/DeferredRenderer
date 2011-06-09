#include "FirstPersonCamera.h"

FirstPersonCamera::FirstPersonCamera()
	: PerspectiveCamera(), _xRot(0.0f), _yRot(0.0f)
{
	UpdateProjection();
}

FirstPersonCamera::FirstPersonCamera(float nearClip, float farClip, float fov, float aspect)
	: PerspectiveCamera(nearClip, farClip, fov, aspect), _xRot(0.0f), _yRot(0.0f)
{
	UpdateProjection();
}

float FirstPersonCamera::GetXRotation()
{
	return _xRot;
}

float FirstPersonCamera::GetYRotation()
{
	return _yRot;
}

void FirstPersonCamera::SetXRotation(float xRot)
{
	_xRot = XMScalarModAngle(xRot);
	SetOrientation(XMQuaternionRotationRollPitchYaw(_xRot, _yRot, 0.0f));
}

void FirstPersonCamera::SetYRotation(float yRot)
{
	_yRot = clamp(yRot, -PiOver2, PiOver2);
	SetOrientation(XMQuaternionRotationRollPitchYaw(_yRot, _xRot, 0.0f));
}

void FirstPersonCamera::SetRotation(float xRot, float yRot)
{
	_xRot = XMScalarModAngle(xRot);
	_yRot = clamp(yRot, -PiOver2, PiOver2);
	SetOrientation(XMQuaternionRotationRollPitchYaw(_yRot, _xRot, 0.0f));
}