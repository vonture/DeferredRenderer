#pragma once

#include "PCH.h"
#include "xnaCollision.h"

class Camera
{
private:
	XMFLOAT4X4 _prevWorld;
	XMFLOAT4X4 _prevView;
	XMFLOAT4X4 _prevProj;
	XMFLOAT4X4 _prevViewProj;
	XMFLOAT4X4 _prevInvViewProj;

	XMFLOAT4X4 _world;
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _proj;
	XMFLOAT4X4 _viewProj;
	XMFLOAT4X4 _invViewProj;
	float _nearClip;
	float _farClip;

	void worldMatrixChanged();
	void projMatrixChanged();
	void viewMatrixChanged();

protected:
	void UpdateProjection();

	virtual void BuildProjection(XMMATRIX* outProj, float nearClip, float farClip);
public:
	Camera();
	Camera(float nearClip, float farClip);
	virtual ~Camera();

	void SetLookAt(const XMFLOAT3 &eye, const XMFLOAT3 &lookAt, const XMFLOAT3 &up);
	void SetLookTo(const XMFLOAT3 &eye, const XMFLOAT3 &lookTo, const XMFLOAT3 &up);

	void SetPosition(const XMFLOAT3& pos);
	XMFLOAT3 GetPosition() const;

	void SetOrientation(const XMFLOAT4& newOrientation);
	XMFLOAT4 GetOrientation() const;

	void SetClips(float nearClip, float farClip);	
	float GetNearClip() const;
	float GetFarClip() const;	

	const XMFLOAT4X4& GetView() const;	
	const XMFLOAT4X4& GetViewProjection() const;
	const XMFLOAT4X4& GetInverseViewProjection() const;

	const XMFLOAT4X4& GetPreviousView() const;
	const XMFLOAT4X4& GetPreviousProjection() const;
	const XMFLOAT4X4& GetPreviousViewProjection() const;
	const XMFLOAT4X4& GetPreviousInverseViewProjection() const;

	void SetWorld(const XMFLOAT4X4& world);
	const XMFLOAT4X4& GetWorld() const;
	const XMFLOAT4X4& GetPreviousWorld() const;

	void SetProjection(const XMFLOAT4X4& proj);
	const XMFLOAT4X4& GetProjection() const;

	void StoreMatrices();

	XMFLOAT3 GetForward() const;
	XMFLOAT3 GetBackward() const;
	XMFLOAT3 GetRight() const;
	XMFLOAT3 GetLeft() const;
	XMFLOAT3 GetUp() const;
	XMFLOAT3 GetDown() const;

	Ray UnprojectRay(const XMFLOAT2& screenPos, const XMFLOAT2& viewPortSize) const;
	XMFLOAT3 UnprojectPosition(const XMFLOAT3& screenPos, const XMFLOAT2& viewPortSize) const;
	
	XMFLOAT3 ProjectPosition(const XMFLOAT3& worldPos);

	Frustum CreateFrustum() const;
};