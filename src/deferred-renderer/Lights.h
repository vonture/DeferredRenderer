#pragma once

#include "PCH.h"

_DECLSPEC_ALIGN_16_ struct AmbientLight
{
	XMFLOAT3 Color;	
};

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
	XMFLOAT3 Direction;
	FLOAT Length;
	FLOAT Angle;
	XMFLOAT3 Color;
};