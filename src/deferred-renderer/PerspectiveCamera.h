#pragma once

#include "PCH.h"
#include "Camera.h"

class PerspectiveCamera : public Camera
{
private:
	float _fov;
	float _aspect;

protected:
	void BuildProjection(XMMATRIX* outProj, float nearClip, float farClip);

public:
	PerspectiveCamera();
	PerspectiveCamera(float nearClip, float farClip, float fov, float aspect);
	virtual ~PerspectiveCamera();

	float GetFieldOfView();
	void SetFieldOfView(float fov);

	float GetAspectRatio();
	void SetAspectRatio(float aspect);
};
