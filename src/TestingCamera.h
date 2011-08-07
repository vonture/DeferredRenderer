#pragma once

#include "PCH.h"
#include "FirstPersonCamera.h"

class TestingCamera : public FirstPersonCamera
{
private:
	float _moveSpeed;
	float _rotSpeed;

public:
	TestingCamera();
	TestingCamera(float nearClip, float farClip, float fov, float aspect);

	float GetMovementSpeed() const;
	void SetMovementSpeed(float speed);

	float GetRotationSpeed() const;
	void SetRotationSpeed(float rotSpeed);
};