#pragma once

#include "Defines.h"

_DECLSPEC_ALIGN_16_ struct PointLight
{	
	XMFLOAT3 Position;
	float Radius;
	XMFLOAT3 Color;	
};

_DECLSPEC_ALIGN_16_ struct DirectionalLight
{	
	XMFLOAT3 Direction;
	XMFLOAT3 Color;
};

_DECLSPEC_ALIGN_16_ struct SpotLight
{	
	XMFLOAT3 Position;
	XMFLOAT4 Direction;
	FLOAT Length;
	FLOAT Radius;
	XMFLOAT3 Color;
};
/*
class Light
{
private:
	XMVECTOR _color;
	float _intensity;

public:
	Light(const XMVECTOR& color, float intensity) 
		: _color(color), _intensity(max(intensity, 0.0f))
	{
	}

	const XMVECTOR& GetColor()
	{
		return _color;
	}
	
	void SetColor(const XMVECTOR& color)
	{
		_color = color;
	}
	
	float GetItensity()
	{
		return _intensity;
	}

	void SetIntensity(float intensity)
	{
		_intensity = max(intensity, 0.0f);
	}
};

class DirectionalLight : public Light
{
private:
	XMVECTOR _direction;

public:
	DirectionalLight(const XMVECTOR& color, float intensity, const XMVECTOR& direction)
		: Light(color, intensity), _direction(direction)
	{
		_direction = XMVector3Normalize(direction);
	}

	const XMVECTOR& GetDirection()
	{
		return _direction;
	}

	void SetDirection(const XMVECTOR& direction)
	{
		_direction = XMVector3Normalize(direction);
	}
};

class PointLight : public Light
{
private:
	XMVECTOR _position;
	float _radius;

public:
	PointLight(const XMVECTOR& color, float intensity, const XMVECTOR& position, float radius)
		: Light(color, intensity), _position(position), _radius(radius)
	{
		_radius = max(_radius, 0.0f);
	}

	const XMVECTOR& GetPosition()
	{
		return _position;
	}

	void SetPosition(const XMVECTOR& position)
	{
		_position = position;
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
*/