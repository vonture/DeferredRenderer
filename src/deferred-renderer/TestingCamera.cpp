#include "PCH.h"
#include "TestingCamera.h"

TestingCamera::TestingCamera()
	: FirstPersonCamera()
{
	SetMovementSpeed(10.0f);
	SetRotationSpeed(0.002f);
	SetRollSpeed(1.0f);
}

TestingCamera::TestingCamera(float nearClip, float farClip, float fov, float aspect)
	: FirstPersonCamera(nearClip, farClip, fov, aspect)
{
	SetMovementSpeed(10.0f);
	SetRotationSpeed(0.002f);
	SetRollSpeed(1.0f);
}

TestingCamera::~TestingCamera()
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


float TestingCamera::GetRollSpeed() const
{
	return _rollSpeed;
}

void TestingCamera::SetRollSpeed(float val)
{ 
	_rollSpeed = max(0.0f, val);
}