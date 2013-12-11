#pragma once

#include "PCH.h"

struct ParticleVertex
{
    XMFLOAT3 Position;
    XMFLOAT3 PreviousPosition;
    XMFLOAT4 Color;
    float Radius;
    float PreviousRadius;
    float Rotation;
    float PreviousRotation;
};

struct Particle
{
    XMFLOAT3 Position;
    XMFLOAT3 PreviousPosition;
    XMFLOAT3 Velocity;
    float SpeedPerc;
    XMFLOAT4 Color;
    float Radius;
    float PreviousRadius;
    float Scale;
    float Life;
    float Rotation;
    float PreviousRotation;
    float RotationRate;
};