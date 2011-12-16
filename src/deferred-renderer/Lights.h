#pragma once

#include "PCH.h"
#include "IDragable.h"

class Light
{
private:
	XMFLOAT3 _color;
	float _brightness;

public:
	Light(const XMFLOAT3& color, float brightness);

	const XMFLOAT3& GetColor() const { return _color; }
	void SetColor(const XMFLOAT3& col) { _color = col; }

	float GetBrightness() const { return _brightness; }
	void SetBrightness(float brightness)  { _brightness = brightness; }

	XMFLOAT3 GetMultipliedColor() const;

	void MergeColor(Light* otherLight);
};

class AmbientLight : public Light
{
public:
	AmbientLight(const XMFLOAT3& color, float brightness);
};

class PointLight : public Light, public IDragable
{	
private:
	XMFLOAT3 _position;
	float _radius;

public:
	PointLight(const XMFLOAT3& pos, float radius, const XMFLOAT3& color, float brightness);

	const XMFLOAT3& GetPosition() const { return _position; }
	void SetPosition(const XMFLOAT3& pos) { _position = pos; }

	float GetRadius() const { return _radius; }
	void SetRadius(float rad)  { _radius = rad; }

	void FillBoundingObjectSet(BoundingObjectSet* set);
	bool RayIntersect(const Ray& ray, float* dist);
};

struct DirectionalLight : public Light
{	
private:
	XMFLOAT3 _direction;

public:
	DirectionalLight(const XMFLOAT3& dir, const XMFLOAT3& color, float brightness);

	const XMFLOAT3& GetDirection() const { return _direction; }
	void SetDirection(const XMFLOAT3& dir) { _direction = dir; }
};

struct SpotLight : public Light, public IDragable
{	
private:
	XMFLOAT3 _position;
	XMFLOAT3 _direction;
	float _length;
	float _angle;

public:
	SpotLight(const XMFLOAT3& pos, const XMFLOAT3& dir, float length, float angle, const XMFLOAT3& color,
		float brightness);

	const XMFLOAT3& GetPosition() const { return _position; }
	void SetPosition(const XMFLOAT3& pos) { _position = pos; }

	const XMFLOAT3& GetDirection() const { return _direction; }
	void SetDirection(const XMFLOAT3& dir) { _direction = dir; }

	float GetLength() const { return _length; }
	void SetLengths(float len)  { _length = len; }

	float GetAngle() const { return _angle; }
	void SetAngle(float angle) { _angle = angle; }

	void FillBoundingObjectSet(BoundingObjectSet* set);
	bool RayIntersect(const Ray& ray, float* dist);
};