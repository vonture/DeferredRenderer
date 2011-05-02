#pragma once

#include "DXUT.h"

class BoundingFrustum
{
private:
	D3DXPLANE _planes[6];
	D3DXVECTOR3 _corners[8];

	void updateValues(const D3DXMATRIX& matrix)
	{
		_planes[0].a = matrix._14 + matrix._11;
		_planes[0].b = matrix._24 + matrix._21;
		_planes[0].c = matrix._34 + matrix._31;
		_planes[0].d = matrix._44 + matrix._41;
 
		// Right plane
		_planes[1].a = matrix._14 - matrix._11;
		_planes[1].b = matrix._24 - matrix._21;
		_planes[1].c = matrix._34 - matrix._31;
		_planes[1].d = matrix._44 - matrix._41;
 
		// Top plane
		_planes[2].a = matrix._14 - matrix._12;
		_planes[2].b = matrix._24 - matrix._22;
		_planes[2].c = matrix._34 - matrix._32;
		_planes[2].d = matrix._44 - matrix._42;
 
		// Bottom plane
		_planes[3].a = matrix._14 + matrix._12;
		_planes[3].b = matrix._24 + matrix._22;
		_planes[3].c = matrix._34 + matrix._32;
		_planes[3].d = matrix._44 + matrix._42;
 
		// Near plane
		_planes[4].a = matrix._13;
		_planes[4].b = matrix._23;
		_planes[4].c = matrix._33;
		_planes[4].d = matrix._43;
 
		// Far plane
		_planes[5].a = matrix._14 - matrix._13;
		_planes[5].b = matrix._24 - matrix._23;
		_planes[5].c = matrix._34 - matrix._33;
		_planes[5].d = matrix._44 - matrix._43;
 
		// Normalize planes
		for (int i = 0; i < 6; i++)
		{
			D3DXPlaneNormalize(&_planes[i], &_planes[i]);
		}

		// Generate Points
		D3DXMATRIX viewProjInv;
		D3DXMatrixInverse(&viewProjInv, NULL, &matrix);
		
		D3DXVECTOR3 pt;

		pt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		D3DXVec3TransformCoord(&_corners[0], &pt, &viewProjInv);

		pt = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		D3DXVec3TransformCoord(&_corners[0], &pt, &viewProjInv);

		pt = D3DXVECTOR3(1.0f, 1.0f, 0.0f);
		D3DXVec3TransformCoord(&_corners[0], &pt, &viewProjInv);

		pt = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		D3DXVec3TransformCoord(&_corners[0], &pt, &viewProjInv);

		pt = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		D3DXVec3TransformCoord(&_corners[0], &pt, &viewProjInv);

		pt = D3DXVECTOR3(1.0f, 0.0f, 1.0f);
		D3DXVec3TransformCoord(&_corners[0], &pt, &viewProjInv);

		pt = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
		D3DXVec3TransformCoord(&_corners[0], &pt, &viewProjInv);

		pt = D3DXVECTOR3(0.0f, 1.0f,1.0f);
		D3DXVec3TransformCoord(&_corners[0], &pt, &viewProjInv);
	}

public:
	BoundingFrustum()
	{
		D3DXMATRIX identity;
		D3DXMatrixIdentity(&identity);

		updateValues(identity);
	}

	BoundingFrustum(const D3DXMATRIX& matrix)
	{
		updateValues(matrix);
	}

	void SetMatrix(const D3DXMATRIX& matrix)
	{
		updateValues(matrix);
	}

	void GetCorners(const D3DXVECTOR3* outCorners)
	{
		outCorners = _corners;
	}
};

class BoundingSphere
{
private:
	D3DXVECTOR3 _position;
	float _radius;

public:
	BoundingSphere() : _position(0.0f, 0.0f, 0.0f), _radius(0.0f)
	{
	}

	BoundingSphere(const D3DXVECTOR3& pos, float rad) : _position(pos), _radius(rad)
	{
	}

	const D3DXVECTOR3* GetPosition()
	{
		return &_position;
	}

	void SetPosition(const D3DXVECTOR3& pos)
	{
		_position = pos;
	}

	float GetRadius()
	{
		return _radius;
	}

	void SetRadius(float radius)
	{
		_radius = max(radius, 0.0f);
	}
};

class BoundingBox
{
private:
	D3DXVECTOR3 _min;
	D3DXVECTOR3 _max;

public:
	BoundingBox() : _min(0.0f, 0.0f, 0.0f), _max(0.0f, 0.0f, 0.0f)
	{
	}

	BoundingBox(const D3DXVECTOR3& min, const D3DXVECTOR3& max) : _min(min), _max(max)
	{
	}

	const D3DXVECTOR3* GetMin()
	{
		return &_min;
	}

	void SetMin(const D3DXVECTOR3& min)
	{
		_min = min;
	}

	const D3DXVECTOR3* GetMax()
	{
		return &_max;
	}

	void SetMax(const D3DXVECTOR3& max)
	{
		_max = max;
	}
};