#pragma once

#include "PCH.h"
#include "PerspectiveCamera.h"

class FirstPersonCamera : public PerspectiveCamera
{
private:
	XMFLOAT2 _rotation;

public:
	FirstPersonCamera();
	FirstPersonCamera(float nearClip, float farClip, float fov, float aspect);
	~FirstPersonCamera();

	const XMFLOAT2& GetRotation() const;
	void SetRotation(const XMFLOAT2& rotation);
};