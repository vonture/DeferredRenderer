#pragma once

#include "PCH.h"
#include "BoundingObjectSet.h"
#include "xnaCollision.h"

class IDragable
{
public:
	virtual const XMFLOAT3& GetPosition() const = 0;
	virtual void SetPosition(const XMFLOAT3& pos) = 0;

	//virtual float GetScale() const = 0;
	//virtual void SetScale(float scale) = 0;

	virtual void FillBoundingObjectSet(BoundingObjectSet* set) = 0;
	virtual bool RayIntersect(const Ray& ray, float* dist) = 0;
};