#include "PCH.h"
#include "TestingCamera.h"

TestingCamera::TestingCamera()
	: FirstPersonCamera(), _moveSpeed(10.0f), _rotSpeed(0.002f)
{
}

TestingCamera::TestingCamera(float nearClip, float farClip, float fov, float aspect)
	: FirstPersonCamera(nearClip, farClip, fov, aspect), _moveSpeed(10.0f), _rotSpeed(0.002f)
{
}

float TestingCamera::GetMovementSpeed() const 
{ 
	return _moveSpeed;
}

void TestingCamera::SetMovementSpeed(float speed)
{ 
	_moveSpeed = max(0.0f, speed); 
}

float TestingCamera::GetRotationSpeed() const
{
	return _rotSpeed;
}

void TestingCamera::SetRotationSpeed(float rotSpeed)
{
	_rotSpeed = max(0.0f, rotSpeed);
}