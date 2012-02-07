#pragma once

#include "PCH.h"
#include "PerspectiveCamera.h"

class FirstPersonCamera : public PerspectiveCamera
{
private:
	XMFLOAT2 _rotation;
	float _roll;

public:
	FirstPersonCamera();
	FirstPersonCamera(float nearClip, float farClip, float fov, float aspect);
	~FirstPersonCamera();

	float GetRoll() const;
	const XMFLOAT2& GetRotation() const;
	void SetRotation(const XMFLOAT2& rotation, float roll);
};