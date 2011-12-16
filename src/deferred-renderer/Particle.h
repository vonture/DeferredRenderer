#pragma once

#include "PCH.h"

struct ParticleVertex
{
	XMFLOAT3 Position;
	XMFLOAT4 Color;
	float Radius;
	float Rotation;
};

struct Particle
{
	XMFLOAT3 Position;
	XMFLOAT3 Velocity;
	XMFLOAT4 Color;
	float Radius;
	float Scale;
	float Life;
	float Rotation;
	float RotationRate;
};