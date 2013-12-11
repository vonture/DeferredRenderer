#pragma once

#include "PCH.h"

class Poisson
{
private:
    static const UINT DISTRIBUTION_COUNT = 5;
    static const UINT DISTRIBUTION_SIZES[DISTRIBUTION_COUNT];

    static const XMFLOAT4 Poisson12[12];
    static const XMFLOAT4 Poisson16[16];
    static const XMFLOAT4 Poisson25[25];
    static const XMFLOAT4 Poisson32[32];
    static const XMFLOAT4 Poisson64[64];

public:
    static UINT GetDistributionCount() { return DISTRIBUTION_COUNT; }

    static const XMFLOAT4* GetDistribution(UINT idx);
    static UINT GetDistributionSize(UINT idx);
};