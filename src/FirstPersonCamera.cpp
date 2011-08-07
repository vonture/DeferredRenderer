#include "PCH.h"
#include "FirstPersonCamera.h"

FirstPersonCamera::FirstPersonCamera()
	: PerspectiveCamera(), _rotation(0.0f, 0.0f)
{
	UpdateProjection();
}

FirstPersonCamera::FirstPersonCamera(float nearClip, float farClip, float fov, float aspect)
	: PerspectiveCamera(nearClip, farClip, fov, aspect), _rotation(0.0f, 0.0f)
{
	UpdateProjection();
}

const XMFLOAT2& FirstPersonCamera::GetRotation() const
{
	return _rotation;
}

void FirstPersonCamera::SetRotation(const XMFLOAT2& rotation)
{
	_rotation.x = XMScalarModAngle(rotation.x);
	_rotation.y = clamp(rotation.y, -PiOver2, PiOver2);

	XMVECTOR quat = XMQuaternionRotationRollPitchYaw(_rotation.y, _rotation.x, 0.0f);
	
	XMFLOAT4 newOrientation;
	XMStoreFloat4(&newOrientation, quat);

	SetOrientation(newOrientation);	
}