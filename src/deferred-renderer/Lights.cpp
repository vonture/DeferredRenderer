#include "PCH.h"
#include "Lights.h"

Light::Light(const XMFLOAT3& color, float brightness)
	: _color(color), _brightness(brightness)	
{
}

XMFLOAT3 Light::GetMultipliedColor() const
{
	return XMFLOAT3(_color.x * _brightness, _color.y * _brightness, _color.z * _brightness);
}

void Light::MergeColor(Light* otherLight)
{
	XMVECTOR mergeColor = XMVectorSet(
		_color.x * _brightness + otherLight->_color.x * otherLight->_brightness,
		_color.y * _brightness + otherLight->_color.y * otherLight->_brightness,
		_color.z * _brightness + otherLight->_color.z * otherLight->_brightness,
		1.0f);

	XMStoreFloat3(&_color, XMVector3Normalize(mergeColor));
	XMStoreFloat(&_brightness, XMVector3Length(mergeColor));
}

PointLight::PointLight(const XMFLOAT3& pos, float radius, const XMFLOAT3& color, float brightness)
	: Light(color, brightness), _position(pos), _radius(radius)
{
}

void PointLight::FillBoundingObjectSet(BoundingObjectSet* set)
{
	Sphere s;
	s.Center = _position;
	s.Radius = _radius;

	set->AddSphere(s);
}

bool PointLight::RayIntersect(const Ray& ray, float* dist)
{
	Sphere s;
	s.Center = _position;
	s.Radius = _radius;

	XMVECTOR rayOrigin = XMLoadFloat3(&ray.Origin);
	XMVECTOR rayDir = XMLoadFloat3(&ray.Direction);

	return !Collision::IntersectPointSphere(rayOrigin, &s) &&
		Collision::IntersectRaySphere(rayOrigin, rayDir, &s, dist);
}

AmbientLight::AmbientLight(const XMFLOAT3& color, float brightness)
	: Light(color, brightness)
{
}

DirectionalLight::DirectionalLight(const XMFLOAT3& dir, const XMFLOAT3& color, float brightness)
	: Light(color, brightness), _direction(dir)
{
}

SpotLight::SpotLight(const XMFLOAT3& pos, const XMFLOAT3& dir, float length, float angle, 
	const XMFLOAT3& color, float brightness)
	: Light(color, brightness), _position(pos),_direction(dir), _length(length), _angle(angle)
{
}

void SpotLight::FillBoundingObjectSet(BoundingObjectSet* set)
{
}

bool SpotLight::RayIntersect(const Ray& ray, float* dist)
{
	return false;
}