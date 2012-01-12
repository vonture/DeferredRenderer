#include "PCH.h"
#include "Camera.h"

Camera::Camera()
{
	_nearClip = max(0.1f, EPSILON);
	_farClip = max(_nearClip + EPSILON, 100.0f);

	XMStoreFloat4x4(&_world, XMMatrixIdentity());
	worldMatrixChanged();
}

Camera::Camera(float nearClip, float farClip)
{
	_nearClip = max(nearClip, EPSILON);
	_farClip = max(_nearClip + EPSILON, farClip);

	XMStoreFloat4x4(&_world, XMMatrixIdentity());
	worldMatrixChanged();
}

Camera::~Camera()
{
}

void Camera::BuildProjection(XMMATRIX* outProj, float nearClip, float farClip )
{
	*outProj = XMMatrixOrthographicLH(1.0f, 1.0f, nearClip, farClip);
}

void Camera::worldMatrixChanged()
{	
	XMMATRIX world = XMLoadFloat4x4(&_world);
	XMMATRIX proj = XMLoadFloat4x4(&_proj);

	XMVECTOR det;
	XMMATRIX view = XMMatrixInverse(&det, world);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invViewProj = XMMatrixInverse(&det, viewProj);
	
	XMStoreFloat4x4(&_view, view);
	XMStoreFloat4x4(&_viewProj, viewProj);
	XMStoreFloat4x4(&_invViewProj, invViewProj);
}

void Camera::projMatrixChanged()
{
	XMMATRIX world = XMLoadFloat4x4(&_world);
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX proj = XMLoadFloat4x4(&_proj);

	XMVECTOR det;
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invViewProj = XMMatrixInverse(&det, viewProj);
	XMMATRIX invProj = XMMatrixInverse(&det, proj);
	
	XMVECTOR n = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	n = XMVector4Transform(n, invProj);
	_nearClip = XMVectorGetZ(n * XMVectorReciprocal(XMVectorSplatW(n)));

	XMVECTOR f = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
	f = XMVector4Transform(f, invProj);
	_farClip = XMVectorGetZ(f * XMVectorReciprocal(XMVectorSplatW(f)));

	XMStoreFloat4x4(&_viewProj, viewProj);
	XMStoreFloat4x4(&_invViewProj, invViewProj);
}

void Camera::viewMatrixChanged()
{
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX proj = XMLoadFloat4x4(&_proj);

	XMVECTOR det;
	XMMATRIX world = XMMatrixInverse(&det, view);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invViewProj = XMMatrixInverse(&det, viewProj);

	XMStoreFloat4x4(&_world, world);
	XMStoreFloat4x4(&_viewProj, viewProj);
	XMStoreFloat4x4(&_invViewProj, invViewProj);
}

void Camera::UpdateProjection()
{
	XMMATRIX proj;
	BuildProjection(&proj, _nearClip, _farClip);
	XMStoreFloat4x4(&_proj, proj);

	projMatrixChanged();
}

void Camera::SetLookAt(const XMFLOAT3 &eye, const XMFLOAT3 &lookAt, const XMFLOAT3 &up)
{	
	XMMATRIX view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&lookAt), XMLoadFloat3(&up));
	XMStoreFloat4x4(&_view, view);

	viewMatrixChanged();
}

void Camera::SetLookTo(const XMFLOAT3 &eye, const XMFLOAT3 &lookTo, const XMFLOAT3 &up)
{
	XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&eye), XMLoadFloat3(&lookTo), XMLoadFloat3(&up));
	XMStoreFloat4x4(&_view, view);

	viewMatrixChanged();
}

void Camera::SetPosition(const XMFLOAT3& pos)
{
	_world._41 = pos.x;
	_world._42 = pos.y;
	_world._43 = pos.z;
	
	worldMatrixChanged();
}

XMFLOAT3 Camera::GetPosition() const
{ 
	return XMFLOAT3(_world._41, _world._42, _world._43);
}

void Camera::SetOrientation(const XMFLOAT4& newOrientation)
{
	XMMATRIX world = XMLoadFloat4x4(&_world);
	XMMATRIX newWorld = XMMatrixRotationQuaternion(XMLoadFloat4(&newOrientation));

	newWorld._41 = world._41;
	newWorld._42 = world._42;
	newWorld._43 = world._43;

	XMStoreFloat4x4(&_world, newWorld);

	worldMatrixChanged();
}

XMFLOAT4 Camera::GetOrientation() const 
{
	XMMATRIX world = XMLoadFloat4x4(&_world);
	XMVECTOR quat = XMQuaternionRotationMatrix(world);

	XMFLOAT4 orientation;
	XMStoreFloat4(&orientation, quat);

	return orientation;
}

void Camera::SetClips(float nearClip, float farClip)
{
	_nearClip = max(nearClip, EPSILON);
	_farClip = max(_nearClip + EPSILON, farClip);

	UpdateProjection();
}

float Camera::GetNearClip() const
{
	return _nearClip;
}

float Camera::GetFarClip() const
{
	return _farClip;
}

const XMFLOAT4X4& Camera::GetView() const
{ 
	return _view;
}

const XMFLOAT4X4& Camera::GetProjection() const
{ 
	return _proj; 
}

const XMFLOAT4X4& Camera::GetViewProjection() const
{
	return _viewProj;
}

const XMFLOAT4X4& Camera::GetInverseViewProjection() const
{
	return _invViewProj;
}

const XMFLOAT4X4& Camera::GetPreviousView() const
{
	return _prevView;
}

const XMFLOAT4X4& Camera::GetPreviousProjection() const
{
	return _prevProj;
}

const XMFLOAT4X4& Camera::GetPreviousViewProjection() const
{
	return _prevViewProj;
}

const XMFLOAT4X4& Camera::GetPreviousInverseViewProjection() const
{
	return _prevInvViewProj;
}

void Camera::SetWorld(const XMFLOAT4X4& world)
{
	_world = world;
	worldMatrixChanged();
}

const XMFLOAT4X4& Camera::GetWorld() const
{ 
	return _world;
}

const XMFLOAT4X4& Camera::GetPreviousWorld() const
{
	return _prevWorld;
}

void Camera::SetProjection(const XMFLOAT4X4& proj)
{
	_proj = proj;
	projMatrixChanged();
}

void Camera::StoreMatrices()
{
	_prevWorld = _world;
	_prevView = _view;
	_prevProj = _proj;
	_prevViewProj = _viewProj;
	_prevInvViewProj = _invViewProj;
}

XMFLOAT3 Camera::GetForward() const
{
	return XMFLOAT3(_world._31, _world._32, _world._33);
}

XMFLOAT3 Camera::GetBackward() const
{
	return XMFLOAT3(-_world._31, -_world._32, -_world._33);
}	

XMFLOAT3 Camera::GetRight() const
{
	return XMFLOAT3(_world._11, _world._12, _world._13);
}

XMFLOAT3 Camera::GetLeft() const
{
	return XMFLOAT3(-_world._11, -_world._12, -_world._13);
}

XMFLOAT3 Camera::GetUp() const
{
	return XMFLOAT3(_world._21, _world._22, _world._23);
}

XMFLOAT3 Camera::GetDown() const
{
	return XMFLOAT3(-_world._21, -_world._22, -_world._23);
}

Ray Camera::UnprojectRay(const XMFLOAT2& screenPos, const XMFLOAT2& viewPortSize) const
{
	XMFLOAT2 viewPortPos = XMFLOAT2((screenPos.x / viewPortSize.x) * 2.0f - 1.0f,
		-((screenPos.y / viewPortSize.y) * 2.0f - 1.0f));

	XMVECTOR start = XMVectorSet(viewPortPos.x, viewPortPos.y, 0.0f, 1.0f);
	XMVECTOR end = XMVectorSet(viewPortPos.x, viewPortPos.y, 1.0f, 1.0f);

	XMMATRIX invViewProj = XMLoadFloat4x4(&_invViewProj);

	start = XMVector3TransformCoord(start, invViewProj);
	end = XMVector3TransformCoord(end, invViewProj);
	
	Ray r;
	XMStoreFloat3(&r.Origin, start);
	XMStoreFloat3(&r.Direction, XMVector3Normalize(end - start));

	return r;
}

XMFLOAT3 Camera::UnprojectPosition(const XMFLOAT3& screenPos, const XMFLOAT2& viewPortSize) const
{
	XMVECTOR viewPortPos = XMVectorSet((screenPos.x / viewPortSize.x) * 2.0f - 1.0f,
		-((screenPos.y / viewPortSize.y) * 2.0f - 1.0f), screenPos.z, 1.0f);

	XMMATRIX invViewProj = XMLoadFloat4x4(&_invViewProj);

	XMVECTOR transformed = XMVector3TransformCoord(viewPortPos, invViewProj);

	XMFLOAT3 result;
	XMStoreFloat3(&result, transformed);

	return result;
}

XMFLOAT3 Camera::ProjectPosition(const XMFLOAT3& worldPos)
{
	XMVECTOR worldPosVec = XMLoadFloat3(&worldPos);
	XMMATRIX viewProj = XMLoadFloat4x4(&_viewProj);

	XMVECTOR transformed = XMVector3TransformCoord(worldPosVec, viewProj);

	XMFLOAT3 result;
	XMStoreFloat3(&result, transformed);

	return result;
}

Frustum Camera::CreateFrustum() const
{
	XMMATRIX proj = XMLoadFloat4x4(&_proj);

	Frustum f;
	Collision::ComputeFrustumFromProjection(&f, &proj);

	f.Origin = GetPosition();
	f.Orientation = GetOrientation();
	
	return f;
}

