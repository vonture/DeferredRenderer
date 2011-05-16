#pragma once

#include "Defines.h"

class BoundingFrustum
{
	friend class Intersection;

private:
	XMVECTOR _planes[6];
	XMVECTOR _corners[8];
	XMVECTOR _mid;

	void updateValues(const XMMATRIX& matrix)
	{
		XMVECTOR corners[8] =
		{                                               //                         7--------6
			XMVectorSet( 1.0f, -1.0f, 0.0f, 1.0f),      //                        /|       /|
			XMVectorSet(-1.0f, -1.0f, 0.0f, 1.0f),      //     Y ^               / |      / |
			XMVectorSet( 1.0f,  1.0f, 0.0f, 1.0f),      //     | _              3--------2  |
			XMVectorSet(-1.0f,  1.0f, 0.0f, 1.0f),      //     | /' Z           |  |     |  |
			XMVectorSet( 1.0f, -1.0f, 1.0f, 1.0f),      //     |/               |  5-----|--4
			XMVectorSet(-1.0f, -1.0f, 1.0f, 1.0f),      //     + ---> X         | /      | /
			XMVectorSet( 1.0f,  1.0f, 1.0f, 1.0f),      //                      |/       |/
			XMVectorSet(-1.0f,  1.0f, 1.0f, 1.0f),      //                      1--------0
		};

		for(UINT i = 0; i < 8; ++i)
		{
			_corners[i] = XMVector3TransformCoord(corners[i], matrix);
		}

		_planes[0] = XMPlaneFromPoints(_corners[0], _corners[4], _corners[2]);
		_planes[1] = XMPlaneFromPoints(_corners[1], _corners[3], _corners[5]);
		_planes[2] = XMPlaneFromPoints(_corners[3], _corners[2], _corners[7]);
		_planes[3] = XMPlaneFromPoints(_corners[1], _corners[5], _corners[0]);
		_planes[4] = XMPlaneFromPoints(_corners[5], _corners[7], _corners[4]);
		_planes[5] = XMPlaneFromPoints(_corners[1], _corners[0], _corners[3]);

		// Calculate the mid point
		_mid = XMVector3TransformCoord(XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f), matrix);
	}

public:
	BoundingFrustum()
	{
		updateValues(XMMatrixIdentity());
	}

	BoundingFrustum(const XMMATRIX& matrix)
	{
		updateValues(matrix);
	}

	void SetMatrix(const XMMATRIX& matrix)
	{
		updateValues(matrix);
	}

	void GetCorners(XMVECTOR* outCorners) const
	{
		for (int i = 0; i < 8; i++)
		{
			outCorners[i] = _corners[i];
		}
	}
};

class BoundingSphere
{
	friend class Intersection;

private:
	XMVECTOR _position;
	float _radius;

public:
	BoundingSphere() : _radius(0.0f)
	{
		_position = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	}

	BoundingSphere(const XMVECTOR& pos, float rad) : _position(pos), _radius(rad)
	{
	}

	const XMVECTOR* GetPosition() const
	{
		return &_position;
	}

	void SetPosition(const XMVECTOR& pos)
	{
		_position = pos;
	}

	float GetRadius() const
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
	friend class Intersection;

private:
	XMVECTOR _min;
	XMVECTOR _max;

public:
	BoundingBox()
	{
		_min = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		_max = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	}

	BoundingBox(const XMVECTOR& min, const XMVECTOR& max) : _min(min), _max(max)
	{
	}

	const XMVECTOR* GetMin() const
	{
		return &_min;
	}

	void SetMin(const XMVECTOR& min)
	{
		_min = min;
	}

	const XMVECTOR* GetMax() const
	{
		return &_max;
	}

	void SetMax(const XMVECTOR& max)
	{
		_max = max;
	}

	void GetCorners(XMVECTOR* outCorners) const
	{
		outCorners[0] = XMVectorSet(XMVectorGetX(_min), XMVectorGetY(_min), XMVectorGetZ(_min), 1.0f);
		outCorners[1] = XMVectorSet(XMVectorGetX(_min), XMVectorGetY(_min), XMVectorGetZ(_max), 1.0f);
		outCorners[2] = XMVectorSet(XMVectorGetX(_min), XMVectorGetY(_max), XMVectorGetZ(_min), 1.0f);
		outCorners[3] = XMVectorSet(XMVectorGetX(_min), XMVectorGetY(_max), XMVectorGetZ(_max), 1.0f);
		outCorners[4] = XMVectorSet(XMVectorGetX(_max), XMVectorGetY(_min), XMVectorGetZ(_min), 1.0f);
		outCorners[5] = XMVectorSet(XMVectorGetX(_max), XMVectorGetY(_min), XMVectorGetZ(_max), 1.0f);
		outCorners[6] = XMVectorSet(XMVectorGetX(_max), XMVectorGetY(_max), XMVectorGetZ(_min), 1.0f);
		outCorners[7] = XMVectorSet(XMVectorGetX(_max), XMVectorGetY(_max), XMVectorGetZ(_max), 1.0f);
	}

	static void CreateFromPoints(BoundingBox* outBB, const XMVECTOR* pts, int ptCount)
	{
		if (ptCount	> 0)
		{
			outBB->_min = XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
			outBB->_max = XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);

			for (int i = 0; i < ptCount; i++)
			{
				outBB->_min = XMVectorMin(outBB->_min, pts[i]);
				outBB->_max = XMVectorMax(outBB->_max, pts[i]);
			}
		}
		else
		{
			outBB->_min = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			outBB->_max = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		}
	}

	static void Transform(BoundingBox* outBB, const BoundingBox* inBB, const XMMATRIX* transform)
	{
		XMVECTOR corners[8];
		inBB->GetCorners(&corners[0]);

		for(int i = 0; i < 8; i++)
		{
			corners[i] = XMVector3TransformCoord(corners[i], *transform);
		}

		CreateFromPoints(outBB, corners, 8);
	}

	static void Combine(BoundingBox* outBB, const BoundingBox* inBBFirst, const BoundingBox* inBBSecond)
	{
		outBB->_min = XMVectorMin(inBBFirst->_min, inBBSecond->_min);
		outBB->_min = XMVectorMin(inBBFirst->_max, inBBSecond->_max);
	}
};

class Intersection
{
public:
	static bool Contains(const BoundingFrustum* frust, const BoundingSphere* sphere)
	{
		for (UINT i = 0; i < 6; i++)
		{
			if (XMVectorGetX(XMPlaneDotCoord(frust->_planes[i], sphere->_position)) + sphere->_radius < 0)
			{
				return false;
			}
		}
		return true;
	}

	static bool Contains(const BoundingFrustum* frust, const BoundingBox* box)
	{
		// not a perfect intersection test but much faster than checking each corner and plane

		XMVECTOR bbMid = (box->_min + box->_max) * 0.5f;
		XMVECTOR bbHalfSize = (box->_min - box->_max) * 0.5f;
		float bbRadius = XMVectorGetX(XMVector3Length(bbHalfSize));

		for (UINT i = 0; i < 6; i++)
		{
			if (XMVectorGetX(XMPlaneDotCoord(frust->_planes[i], bbMid)) + bbRadius < 0)
			{
				return false;
			}
		}
		return true;
	}

	static bool Contains(const BoundingSphere* sphere, const XMVECTOR* pt)
	{
		XMVECTOR radiusVec = XMVectorSubtract(sphere->_position, *pt);
		
		return XMVectorGetX(XMVector3LengthSq(radiusVec)) < (sphere->_radius * sphere->_radius);
	}
};