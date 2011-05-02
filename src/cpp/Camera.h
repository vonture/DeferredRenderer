#pragma once

#include "DXUT.h"
#include "BoundingObjects.h"

class Camera
{
private:
	bool _dirty;
	float _near, _far;
	D3DXMATRIX _world, _view, _proj;

	void updateValues()
    {
		D3DXMatrixInverse(&_view, NULL, &_world);
		BuildProjection(&_proj, _near, _far);
    }

protected:
	virtual void BuildProjection(D3DXMATRIX* outProj, float nearClip, float farClip) = 0;

public:
	Camera(float near, float far) : _near(near), _far(far), _dirty(true)
	{
		D3DXMatrixIdentity(&_world);
	}

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
	FirstPersonCamera(float nearClip, float farClip, float fov, float aspect)
		: PerspectiveCamera(nearClip, farClip, fov, aspect)
	{
	}

	const D3DXVECTOR2* GetRotation()
	{
		return &_rotation;
	}

	void SetRotation(const D3DXVECTOR2& rotation)
	{		
		_rotation.x = rotation.x;
		_rotation.y = max(-M_PI_2, min(M_PI_2, rotation.y));

		D3DXVECTOR3 pos = GetPosition();
		
		D3DXMATRIX rot, translate, world;
		D3DXMatrixRotationYawPitchRoll(&rot, _rotation.x, _rotation.y, 0.0f);
		D3DXMatrixTranslation(&translate, pos.x, pos.y, pos.z);
		D3DXMatrixMultiply(&world, &rot, &translate);

		SetWorld(world);
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
}