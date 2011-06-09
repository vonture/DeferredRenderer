#pragma once

#include "Defines.h"
#include "PerspectiveCamera.h"

class FirstPersonCamera : public PerspectiveCamera
{
private:
	float _xRot, _yRot;

public:
	FirstPersonCamera();
	FirstPersonCamera(float nearClip, float farClip, float fov, float aspect);

	float GetXRotation();
	void SetXRotation(float xRot);

	float GetYRotation();
	void SetYRotation(float yRot);

	void SetRotation(float xRot, float yRot);
};