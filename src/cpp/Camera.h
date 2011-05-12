#pragma once

#include "DXUT.h"
#include "BoundingObjects.h"

class Camera
{
private:
	bool _dirty;
	float _nearClip, _farClip;
	D3DXMATRIX _world, _view, _proj;

	void updateValues()
    {
		D3DXMatrixInverse(&_view, NULL, &_world);
		BuildProjection(&_proj, _nearClip, _farClip);

		_dirty = false;
    }

public:
	Camera() 
		: _nearClip(0.1f), _farClip(1000.0f), _dirty(true)
	{
		D3DXMatrixIdentity(&_world);
	}

	Camera(float nearClip, float farClip) 
		: _nearClip(nearClip), _farClip(farClip), _dirty(true)
	{
		D3DXMatrixIdentity(&_world);
	}

	virtual void BuildProjection(D3DXMATRIX* outProj, float nearClip, float farClip) = 0;

	void SetPosition(const D3DXVECTOR3& pos)
	{
		_world._41 = pos.x;
		_world._42 = pos.y;
		_world._43 = pos.z;

		_dirty = true;
	}

	D3DXVECTOR3 GetPosition() const
	{ 
		return D3DXVECTOR3(_world._41, _world._42, _world._43); 
	}

	D3DXVECTOR3 GetForward() const
	{
		return D3DXVECTOR3(_world._31, _world._32, _world._33);
	}

	D3DXVECTOR3 GetBackward() const
	{

		return D3DXVECTOR3(-_world._31, -_world._32, -_world._33);
	}	

	D3DXVECTOR3 GetRight() const
	{
		return D3DXVECTOR3(_world._11, _world._12, _world._13);
	}

	D3DXVECTOR3 GetLeft() const
	{
		return D3DXVECTOR3(-_world._11, -_world._12, -_world._13);
	}

	D3DXVECTOR3 GetUp() const
	{
		return D3DXVECTOR3(_world._21, _world._22, _world._23);
	}

	D3DXVECTOR3 GetDown() const
	{
		return D3DXVECTOR3(-_world._21, -_world._22, -_world._23);
	}

	void SetOrientation(const D3DXQUATERNION& orientation)
	{
		D3DXMATRIX newWorld;
		D3DXMatrixRotationQuaternion(&newWorld, &orientation);

		newWorld._41 = _world._41;
		newWorld._42 = _world._42;
		newWorld._43 = _world._43;
		_world = newWorld;

		_dirty = true;
	}

	D3DXQUATERNION GetOrientation() const
	{ 
		D3DXQUATERNION quat;
		D3DXQuaternionRotationMatrix(&quat, &_world);
		return quat;
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

	const D3DXMATRIX* GetView()
	{ 
		if (_dirty)
		{
			updateValues();
		}

		return &_view;
	}

	const D3DXMATRIX* GetProjection()
	{ 
		if (_dirty)
		{
			updateValues();
		}

		return &_proj; 
	}

	const D3DXMATRIX* GetWorld() 
	{ 
		return &_world;
	}

	void SetWorld(const D3DMATRIX& world)
	{
		_world = world;
	}
};

class PerspectiveCamera : public Camera
{
private:
	float _fov;
	float _aspect;

protected:
	void BuildProjection(D3DXMATRIX* outProj, float nearClip, float farClip)
	{
		D3DXMatrixPerspectiveFovLH(outProj, _fov, _aspect, nearClip, farClip);
	}

public:
	PerspectiveCamera() 
		: Camera(), _fov(1.0f), _aspect(1.0f)
	{
	}

	PerspectiveCamera(float nearClip, float farClip, float fov, float aspect) 
		: Camera(nearClip, farClip), _fov(fov), _aspect(aspect)
	{
	}

	float GetFieldOfView()
	{
		return _fov;
	}
	void SetFieldOfView(float fov)
	{
		_fov = fov;
	}

	float GetAspectRatio()
	{
		return _aspect;
	}
	void SetAspectRatio(float aspect)
	{
		_aspect = aspect;
	}
};

class FirstPersonCamera : public PerspectiveCamera
{
private:
	D3DXVECTOR2 _rotation;

public:
	FirstPersonCamera()
		: PerspectiveCamera(), _rotation(0.0f, 0.0f)
	{
	}

	FirstPersonCamera(float nearClip, float farClip, float fov, float aspect)
		: PerspectiveCamera(nearClip, farClip, fov, aspect), _rotation(0.0f, 0.0f)
	{
	}

	const D3DXVECTOR2 GetRotation()
	{
		return _rotation;
	}

	void SetRotation(const D3DXVECTOR2& rotation)
	{		
		_rotation.x = rotation.x;
		_rotation.y = max(-D3DX_PI / 2.0f, min(D3DX_PI / 2.0f, rotation.y));
				
		D3DXQUATERNION rotQuat;
		D3DXQuaternionRotationYawPitchRoll(&rotQuat, _rotation.x, _rotation.y, 0.0f);
		SetOrientation(rotQuat);
	}
};

class OrthographicCamera : public Camera
{
private:
	D3DXVECTOR2 _size;

protected:
	void BuildProjection(D3DXMATRIX* outProj, float nearClip, float farClip)
	{
		D3DXMatrixOrthoLH(outProj, _size.x, _size.y, nearClip, farClip);
	}

public:
	OrthographicCamera()
		: Camera(), _size(100.0f, 100.0f)
	{
	}

	OrthographicCamera(float nearClip, float farClip, const D3DXVECTOR2& size)
		: Camera(nearClip, farClip), _size(size)
	{
	}

	OrthographicCamera(float nearClip, float farClip, float width, float height)
		: Camera(nearClip, farClip), _size(width, height)
	{
	}

	const D3DXVECTOR2* GetSize()
	{
		return &_size;
	}

	void SetSize(const D3DXVECTOR2& size)
	{		
		D3DXVECTOR2 minSize = D3DXVECTOR2(0.0f, 0.0f);
		D3DXVec2Maximize(&_size, &size, &minSize);
	}
};