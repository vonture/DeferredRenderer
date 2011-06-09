#pragma once

#include "Defines.h"

class Camera
{
private:
	XMMATRIX _world;
	XMMATRIX _view;
	XMMATRIX _proj;
	XMMATRIX _viewProj;
	float _nearClip;
	float _farClip;

	void worldMatrixChanged();

protected:
	void UpdateProjection();

public:
	Camera();
	Camera(float nearClip, float farClip);
	~Camera();

	virtual XMMATRIX BuildProjection(float nearClip, float farClip) = 0;

	void SetLookAt(const XMVECTOR &eye, const XMVECTOR &lookAt, const XMVECTOR &up);
	void SetLookTo(const XMVECTOR &eye, const XMVECTOR &lookTo, const XMVECTOR &up);

	void SetPosition(const XMVECTOR& pos);
	XMVECTOR GetPosition() const;

	void SetOrientation(const XMVECTOR& newOrientation);
	XMVECTOR GetOrientation() const;	

	void SetNearClip(float nearClip);
	float GetNearClip() const;

	void SetFarClip(float farClip);
	float GetFarClip() const;

	const XMMATRIX& GetView() const;

	void SetProjection(const XMMATRIX& proj);
	const XMMATRIX& GetProjection() const;	

	const XMMATRIX& GetViewProjection() const;

	void SetWorld(const XMMATRIX& world);
	const XMMATRIX& GetWorld() const;

	XMVECTOR GetForward() const;
	XMVECTOR GetBackward() const;
	XMVECTOR GetRight() const;
	XMVECTOR GetLeft() const;
	XMVECTOR GetUp() const;
	XMVECTOR GetDown() const;
};