#include "PCH.h"
#include "FirstPersonCamera.h"

FirstPersonCamera::FirstPersonCamera()
    : PerspectiveCamera()
{
    SetRotation(XMFLOAT2(0.0f, 0.0f), 0.0f);
    UpdateProjection();
}

FirstPersonCamera::FirstPersonCamera(float nearClip, float farClip, float fov, float aspect)
    : PerspectiveCamera(nearClip, farClip, fov, aspect)
{
    SetRotation(XMFLOAT2(0.0f, 0.0f), 0.0f);
    UpdateProjection();
}

FirstPersonCamera::~FirstPersonCamera()
{
}

const XMFLOAT2& FirstPersonCamera::GetRotation() const
{
    return _rotation;
}

void FirstPersonCamera::SetRotation(const XMFLOAT2& rotation, float roll)
{
    _rotation.x = XMScalarModAngle(rotation.x);
    _rotation.y = clamp(rotation.y, -PiOver2, PiOver2);
    _roll = XMScalarModAngle(roll);

    XMVECTOR quat = XMQuaternionRotationRollPitchYaw(_rotation.y, _rotation.x, _roll);

    XMFLOAT4 newOrientation;
    XMStoreFloat4(&newOrientation, quat);

    SetOrientation(newOrientation);
}

float FirstPersonCamera::GetRoll() const
{
    return _roll;
}