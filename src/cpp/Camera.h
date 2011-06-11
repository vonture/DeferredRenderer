#pragma once

#include "Defines.h"

class Camera
{
private:
	XMFLOAT4X4 _world;
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _proj;
	XMFLOAT4X4 _viewProj;
	float _nearClip;
	float _farClip;

	void worldMatrixChanged();

protected:
	void UpdateProjection();

public:
	Camera();
	Camera(float nearClip, float farClip);
	~Camera();

	virtual void BuildProjection(XMMATRIX* outProj, float nearClip, float farClip) = 0;

	void SetLookAt(const XMFLOAT3 &eye, const XMFLOAT3 &lookAt, const XMFLOAT3 &up);
	void SetLookTo(const XMFLOAT3 &eye, const XMFLOAT3 &lookTo, const XMFLOAT3 &up);

	void SetPosition(const XMFLOAT3& pos);
	XMFLOAT3 GetPosition() const;

	void SetOrientation(const XMFLOAT4& newOrientation);
	XMFLOAT4 GetOrientation() const;

	void SetNearClip(float nearClip);
	float GetNearClip() const;

	void SetFarClip(float farClip);
	float GetFarClip() const;

	const XMFLOAT4X4& GetView() const;
	const XMFLOAT4X4& GetProjection() const;
	const XMFLOAT4X4& GetViewProjection() const;

	void SetWorld(const XMFLOAT4X4& world);
	const XMFLOAT4X4& GetWorld() const;

	XMFLOAT3 GetForward() const;
	XMFLOAT3 GetBackward() const;
	XMFLOAT3 GetRight() const;
	XMFLOAT3 GetLeft() const;
	XMFLOAT3 GetUp() const;
	XMFLOAT3 GetDown() const;
};