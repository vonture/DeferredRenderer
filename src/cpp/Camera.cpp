#include "Camera.h"

Camera::Camera()
	: _nearClip(0.1f), _farClip(1000.0f)
{
	_world = XMMatrixIdentity();
	worldMatrixChanged();
}

Camera::Camera(float nearClip, float farClip) 
		: _nearClip(nearClip), _farClip(farClip)
{
	_world = XMMatrixIdentity();
	worldMatrixChanged();
}

Camera::~Camera()
{
}

void Camera::worldMatrixChanged()
{	
	XMVECTOR det;
	_view = XMMatrixInverse(&det, _world);
	_viewProj = XMMatrixMultiply(_view, _proj);
}

void Camera::UpdateProjection()
{
	_proj = BuildProjection(_nearClip, _farClip);
	_viewProj = XMMatrixMultiply(_view, _proj);
}

void Camera::SetLookAt(const XMVECTOR &eye, const XMVECTOR &lookAt, const XMVECTOR &up)
{		
	_view = XMMatrixLookAtLH(eye, lookAt, up);

	XMVECTOR det;
	_world = XMMatrixInverse(&det, _view);
	_viewProj = XMMatrixMultiply(_view, _proj);
}

void Camera::SetLookTo(const XMVECTOR &eye, const XMVECTOR &lookTo, const XMVECTOR &up)
{
	_view = XMMatrixLookToLH(eye, lookTo, up);

	XMVECTOR det;
	_world = XMMatrixInverse(&det, _view);
	_viewProj = XMMatrixMultiply(_view, _proj);
}

void Camera::SetPosition(const XMVECTOR& pos)
{
	_world._41 = XMVectorGetX(pos);
	_world._42 = XMVectorGetY(pos);
	_world._43 = XMVectorGetZ(pos);

	worldMatrixChanged();
}

XMVECTOR Camera::GetPosition() const
{ 
	XMFLOAT3 pos = XMFLOAT3(_world._41, _world._42, _world._43);
	return XMLoadFloat3(&pos);
}

void Camera::SetOrientation(const XMVECTOR& newOrientation)
{
	XMMATRIX newWorld = XMMatrixRotationQuaternion(newOrientation);

	newWorld._41 = _world._41;
	newWorld._42 = _world._42;
	newWorld._43 = _world._43;
	_world = newWorld;

	worldMatrixChanged();
}

XMVECTOR Camera::GetOrientation() const 
{
	return XMQuaternionRotationMatrix(_world);
}

void Camera::SetNearClip(float nearClip)
{
	_nearClip = nearClip;
}

float Camera::GetNearClip() const
{
	return _nearClip;
}

void Camera::SetFarClip(float farClip)
{
	_farClip = farClip;
}

float Camera::GetFarClip() const
{
	return _farClip;
}

const XMMATRIX& Camera::GetView() const
{ 
	return _view;
}

void Camera::SetProjection(const XMMATRIX& proj)
{
	_proj = proj;
	_viewProj = XMMatrixMultiply(_view, _proj);
}

const XMMATRIX& Camera::GetProjection() const
{ 
	return _proj; 
}

const XMMATRIX& Camera::GetViewProjection() const
{
	return _viewProj;
}

void Camera::SetWorld(const XMMATRIX& world)
{
	_world = world;
	worldMatrixChanged();
}

const XMMATRIX& Camera::GetWorld() const
{ 
	return _world;
}

XMVECTOR Camera::GetForward() const
{
	XMFLOAT3 forward = XMFLOAT3(_world._31, _world._32, _world._33);
	return XMLoadFloat3(&forward);
}

XMVECTOR Camera::GetBackward() const
{
	XMFLOAT3 backward = XMFLOAT3(-_world._31, -_world._32, -_world._33);
	return XMLoadFloat3(&backward);
}	

XMVECTOR Camera::GetRight() const
{
	XMFLOAT3 right = XMFLOAT3(_world._11, _world._12, _world._13);
	return XMLoadFloat3(&right);
}

XMVECTOR Camera::GetLeft() const
{
	XMFLOAT3 left = XMFLOAT3(-_world._11, -_world._12, -_world._13);
	return XMLoadFloat3(&left);
}

XMVECTOR Camera::GetUp() const
{
	XMFLOAT3 up = XMFLOAT3(_world._21, _world._22, _world._23);
	return XMLoadFloat3(&up);
}

XMVECTOR Camera::GetDown() const
{
	XMFLOAT3 down = XMFLOAT3(-_world._21, -_world._22, -_world._23);
	return XMLoadFloat3(&down);
}