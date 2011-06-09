#pragma once

#include "Defines.h"
#include "Camera.h"

class PerspectiveCamera : public Camera
{
private:
	float _fov;
	float _aspect;

protected:
	XMMATRIX BuildProjection(float nearClip, float farClip);

public:
	PerspectiveCamera();
	PerspectiveCamera(float nearClip, float farClip, float fov, float aspect);

	float GetFieldOfView();
	void SetFieldOfView(float fov);

	float GetAspectRatio();
	void SetAspectRatio(float aspect);
};
