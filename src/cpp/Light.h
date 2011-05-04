#pragma once

#include "DXUT.h"

class Light
{
private:
	D3DXCOLOR _color;
	float _intensity;

public:
	Light(const D3DXCOLOR& color, float intensity) : _color(color), _intensity(max(intensity, 0.0f))
	{
	}

	const D3DXCOLOR* GetColor()
	{
		return &_color;
	}
	
	void SetColor(const D3DXCOLOR& color)
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
	D3DXVECTOR3 _direction;

public:
	DirectionalLight(const D3DXCOLOR& color, float intensity, const D3DXVECTOR3& direction)
		: Light(color, intensity), _direction(direction)
	{
		D3DXVec3Normalize(&_direction, &_direction);
	}

	const D3DXVECTOR3* GetDirection()
	{
		return &_direction;
	}

	void SetDirection(const D3DXVECTOR3& direction)
	{
		D3DXVec3Normalize(&_direction, &direction);
	}
};

class PointLight : public Light
{
private:
	D3DXVECTOR3 _position;
	float _radius;

public:
	PointLight(const D3DXCOLOR& color, float intensity, const D3DXVECTOR3& position, float radius)
		: Light(color, intensity), _position(position), _radius(radius)
	{
		_radius = max(_radius, 0.0f);
	}

	const D3DXVECTOR3* GetPosition()
	{
		return &_position;
	}

	void SetPosition(const D3DXVECTOR3& position)
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