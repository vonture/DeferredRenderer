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

	void worldMatrixChanged()
    {
		XMVECTOR det;
		_view = XMMatrixInverse(&det, _world);
		_viewProj = XMMatrixMultiply(_view, _proj);
    }

protected:
	void UpdateProjection()
	{
		_proj = BuildProjection(_nearClip, _farClip);
		_viewProj = XMMatrixMultiply(_view, _proj);
	}

public:
	Camera() 
		: _nearClip(0.1f), _farClip(1000.0f)
	{
		_world = XMMatrixIdentity();
		worldMatrixChanged();
	}

	Camera(float nearClip, float farClip) 
		: _nearClip(nearClip), _farClip(farClip)
	{
		_world = XMMatrixIdentity();
		worldMatrixChanged();
	}

	virtual XMMATRIX BuildProjection(float nearClip, float farClip) = 0;

	void SetLookAt(const XMFLOAT3 &eye, const XMFLOAT3 &lookAt, const XMFLOAT3 &up)
	{
		XMVECTOR det;
		_view = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&lookAt), XMLoadFloat3(&up));
		_world = XMMatrixInverse(&det, _view);
		_viewProj = XMMatrixMultiply(_view, _proj);
	}

	void SetPosition(const XMVECTOR& pos)
	{
		_world._41 = XMVectorGetX(pos);
		_world._42 = XMVectorGetY(pos);
		_world._43 = XMVectorGetZ(pos);

		worldMatrixChanged();
	}

	XMVECTOR GetPosition() const
	{ 
		XMFLOAT3 pos = XMFLOAT3(_world._41, _world._42, _world._43);
		return XMLoadFloat3(&pos);
	}

	XMVECTOR GetForward() const
	{
		XMFLOAT3 forward = XMFLOAT3(_world._31, _world._32, _world._33);
		return XMLoadFloat3(&forward);
	}

	XMVECTOR GetBackward() const
	{
		XMFLOAT3 backward = XMFLOAT3(-_world._31, -_world._32, -_world._33);
		return XMLoadFloat3(&backward);
	}	

	XMVECTOR GetRight() const
	{
		XMFLOAT3 right = XMFLOAT3(_world._11, _world._12, _world._13);
		return XMLoadFloat3(&right);
	}

	XMVECTOR GetLeft() const
	{
		XMFLOAT3 left = XMFLOAT3(-_world._11, -_world._12, -_world._13);
		return XMLoadFloat3(&left);
	}

	XMVECTOR GetUp() const
	{
		XMFLOAT3 up = XMFLOAT3(_world._21, _world._22, _world._23);
		return XMLoadFloat3(&up);
	}

	XMVECTOR GetDown() const
	{
		XMFLOAT3 down = XMFLOAT3(-_world._21, -_world._22, -_world._23);
		return XMLoadFloat3(&down);
	}

	void SetOrientation(const XMVECTOR& newOrientation)
	{
		XMMATRIX newWorld = XMMatrixRotationQuaternion(newOrientation);

		newWorld._41 = _world._41;
		newWorld._42 = _world._42;
		newWorld._43 = _world._43;
		_world = newWorld;

		worldMatrixChanged();
	}

	XMVECTOR GetOrientation() const
	{ 
		return XMQuaternionRotationMatrix(_world);
	}

	void SetNearClip(float nearClip)
	{
		_nearClip = nearClip;
	}

	float GetNearClip() const
	{
		return _nearClip;
	}

	void SetFarClip(float farClip)
	{
		_farClip = farClip;
	}

	float GetFarClip() const
	{
		return _farClip;
	}

	const XMMATRIX& GetView()
	{ 
		return _view;
	}

	const XMMATRIX& GetProjection()
	{ 
		return _proj; 
	}

	void SetProjection(const XMMATRIX& proj)
	{
		_proj = proj;
		_viewProj = XMMatrixMultiply(_view, _proj);
	}

	const XMMATRIX& GetViewProjection()
	{ 
		return _viewProj; 
	}

	const XMMATRIX& GetWorld() 
	{ 
		return _world;
	}

	void SetWorld(const XMMATRIX& world)
	{
		_world = world;
		worldMatrixChanged();
	}
};

class PerspectiveCamera : public Camera
{
private:
	float _fov;
	float _aspect;

protected:
	XMMATRIX BuildProjection(float nearClip, float farClip)
	{
		return XMMatrixPerspectiveFovLH(_fov, _aspect, nearClip, farClip);
	}

public:
	PerspectiveCamera() 
		: Camera(), _fov(1.0f), _aspect(1.0f)
	{
		UpdateProjection();
	}

	PerspectiveCamera(float nearClip, float farClip, float fov, float aspect) 
		: Camera(nearClip, farClip), _fov(fov), _aspect(aspect)
	{
		UpdateProjection();
	}

	float GetFieldOfView()
	{
		return _fov;
	}

	void SetFieldOfView(float fov)
	{
		_fov = fov;
		UpdateProjection();
	}

	float GetAspectRatio()
	{
		return _aspect;
	}

	void SetAspectRatio(float aspect)
	{
		_aspect = aspect;
		UpdateProjection();
	}
};

class FirstPersonCamera : public PerspectiveCamera
{
private:
	float _xRot, _yRot;

public:
	FirstPersonCamera()
		: PerspectiveCamera(), _xRot(0.0f), _yRot(0.0f)
	{
		UpdateProjection();
	}

	FirstPersonCamera(float nearClip, float farClip, float fov, float aspect)
		: PerspectiveCamera(nearClip, farClip, fov, aspect), _xRot(0.0f), _yRot(0.0f)
	{
		UpdateProjection();
	}

	float GetXRotation()
	{
		return _xRot;
	}

	float GetYRotation()
	{
		return _yRot;
	}

	void SetXRotation(float xRot)
	{
		_xRot = XMScalarModAngle(xRot);
		SetOrientation(XMQuaternionRotationRollPitchYaw(_xRot, _yRot, 0.0f));
	}

	void SetYRotation(float yRot)
	{
		_yRot = clamp(yRot, -PiOver2, PiOver2);
		SetOrientation(XMQuaternionRotationRollPitchYaw(_yRot, _xRot, 0.0f));
	}

	void SetRotation(float xRot, float yRot)
	{
		_xRot = XMScalarModAngle(xRot);
		_yRot = clamp(yRot, -PiOver2, PiOver2);
		SetOrientation(XMQuaternionRotationRollPitchYaw(_yRot, _xRot, 0.0f));
	}
};

class OrthographicCamera : public Camera
{
private:
	float _xMin;
    float _xMax;
    float _yMin;
    float _yMax;

protected:
	XMMATRIX BuildProjection(float nearClip, float farClip)
	{
		return XMMatrixOrthographicOffCenterLH(_xMin, _xMax, _yMin, _yMax, nearClip, farClip);
	}

public:
	OrthographicCamera()
		: Camera(), _xMin(0.0f), _yMin(0.0f), _xMax(1.0f), _yMax(1.0f)
	{
		UpdateProjection();
	}

	OrthographicCamera(float nearClip, float farClip, float minX, float minY, float maxX, float maxY)
		: Camera(nearClip, farClip), _xMin(minX), _yMin(minY), _xMax(maxX), _yMax(maxY)
	{
		UpdateProjection();
	}

    float GetMinX() const { return _xMin; };
    float GetMinY() const { return _yMin; };
    float GetMaxX() const { return _xMax; };
    float GetMaxY() const { return _yMax; };

    void SetMinX(float minX)
	{
		_xMin = minX;
		UpdateProjection();
	}

    void SetMinY(float minY)
	{
		_yMin = minY;
		UpdateProjection();
	}

    void SetMaxX(float maxX)
	{
		_xMax = maxX;
		UpdateProjection();
	}

    void SetMaxY(float maxY)
	{
		_yMax = maxY;
		UpdateProjection();
	}
};