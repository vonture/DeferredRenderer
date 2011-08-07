#include "PCH.h"
#include "Camera.h"

Camera::Camera()
{
	SetClips(0.1f, 100.0f);

	XMStoreFloat4x4(&_world, XMMatrixIdentity());
	worldMatrixChanged();
}

Camera::Camera(float nearClip, float farClip)
{
	SetClips(nearClip, farClip);

	XMStoreFloat4x4(&_world, XMMatrixIdentity());
	worldMatrixChanged();
}

void Camera::worldMatrixChanged()
{	
	XMMATRIX world = XMLoadFloat4x4(&_world);
	XMMATRIX proj = XMLoadFloat4x4(&_proj);

	XMVECTOR det;
	XMMATRIX view = XMMatrixInverse(&det, world);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	XMStoreFloat4x4(&_view, view);
	XMStoreFloat4x4(&_viewProj, viewProj);
}

void Camera::UpdateProjection()
{
	XMMATRIX view = XMLoadFloat4x4(&_view);

	XMMATRIX proj;
	BuildProjection(&proj, _nearClip, _farClip);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	XMStoreFloat4x4(&_proj, proj);
	XMStoreFloat4x4(&_viewProj, viewProj);
}

void Camera::SetLookAt(const XMFLOAT3 &eye, const XMFLOAT3 &lookAt, const XMFLOAT3 &up)
{		
	XMMATRIX proj = XMLoadFloat4x4(&_proj);

	XMMATRIX view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&lookAt), XMLoadFloat3(&up));

	XMVECTOR det;
	XMMATRIX world = XMMatrixInverse(&det, view);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	XMStoreFloat4x4(&_world, world);
	XMStoreFloat4x4(&_view, view);
	XMStoreFloat4x4(&_viewProj, viewProj);
}

void Camera::SetLookTo(const XMFLOAT3 &eye, const XMFLOAT3 &lookTo, const XMFLOAT3 &up)
{
	XMMATRIX proj = XMLoadFloat4x4(&_proj);

	XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&eye), XMLoadFloat3(&lookTo), XMLoadFloat3(&up));

	XMVECTOR det;
	XMMATRIX world = XMMatrixInverse(&det, view);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	XMStoreFloat4x4(&_world, world);
	XMStoreFloat4x4(&_view, view);
	XMStoreFloat4x4(&_viewProj, viewProj);
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
	_farClip = max(_nearClip, farClip);
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

void Camera::SetWorld(const XMFLOAT4X4& world)
{
	_world = world;
	worldMatrixChanged();
}

const XMFLOAT4X4& Camera::GetWorld() const
{ 
	return _world;
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