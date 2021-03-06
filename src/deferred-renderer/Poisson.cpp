#include "PCH.h"
#include "Poisson.h"

const UINT Poisson::DISTRIBUTION_SIZES[DISTRIBUTION_COUNT] = { 12, 16, 25, 32, 64 };

const XMFLOAT4 Poisson::Poisson12[12] =
{
    XMFLOAT4(-0.326212f, -0.40581f, 0.0f, 0.0f),
    XMFLOAT4(-0.840144f, -0.07358f, 0.0f, 0.0f),
    XMFLOAT4(-0.840144f, 0.457137f, 0.0f, 0.0f),
    XMFLOAT4(-0.203345f, 0.620716f, 0.0f, 0.0f),
    XMFLOAT4(0.96234f, -0.194983f, 0.0f, 0.0f),
    XMFLOAT4(0.473434f, -0.480026f, 0.0f, 0.0f),
    XMFLOAT4(0.519456f, 0.767022f, 0.0f, 0.0f),
    XMFLOAT4(0.185461f, -0.893124f, 0.0f, 0.0f),
    XMFLOAT4(0.507431f, 0.064425f, 0.0f, 0.0f),
    XMFLOAT4(0.89642f, 0.412458f, 0.0f, 0.0f),
    XMFLOAT4(-0.32194f, -0.932615f, 0.0f, 0.0f),
    XMFLOAT4(-0.791559f, -0.59771f, 0.0f, 0.0f),
};

const XMFLOAT4 Poisson::Poisson16[16] =
{
    XMFLOAT4(0.2069841f, 0.2245269f, 0.0f, 0.0f),
    XMFLOAT4(0.525808f, -0.2310817f, 0.0f, 0.0f),
    XMFLOAT4(0.1383943f, 0.9056122f, 0.0f, 0.0f),
    XMFLOAT4(0.6633009f, 0.5129843f, 0.0f, 0.0f),
    XMFLOAT4(-0.4002792f, 0.3727058f, 0.0f, 0.0f),
    XMFLOAT4(0.07912822f, -0.6512921f, 0.0f, 0.0f),
    XMFLOAT4(-0.7726067f, -0.5151217f, 0.0f, 0.0f),
    XMFLOAT4(-0.3843193f, -0.1494132f, 0.0f, 0.0f),
    XMFLOAT4(-0.9107782f, 0.2500633f, 0.0f, 0.0f),
    XMFLOAT4(0.6940153f, -0.7098927f, 0.0f, 0.0f),
    XMFLOAT4(-0.1964669f, -0.379389f, 0.0f, 0.0f),
    XMFLOAT4(-0.4769286f, 0.1840863f, 0.0f, 0.0f),
    XMFLOAT4(-0.247328f, -0.8798459f, 0.0f, 0.0f),
    XMFLOAT4(0.4206541f, -0.714772f, 0.0f, 0.0f),
    XMFLOAT4(0.5829348f, -0.09794202f, 0.0f, 0.0f),
    XMFLOAT4(0.3691838f, 0.4140642f, 0.0f, 0.0f),
};

const XMFLOAT4 Poisson::Poisson25[25] =
{
    XMFLOAT4(-0.978698f, -0.0884121f, 0.0f, 0.0f),
    XMFLOAT4(-0.841121f, 0.521165f, 0.0f, 0.0f),
    XMFLOAT4(-0.71746f, -0.50322f, 0.0f, 0.0f),
    XMFLOAT4(-0.702933f, 0.903134f, 0.0f, 0.0f),
    XMFLOAT4(-0.663198f, 0.15482f, 0.0f, 0.0f),
    XMFLOAT4(-0.495102f, -0.232887f, 0.0f, 0.0f),
    XMFLOAT4(-0.364238f, -0.961791f, 0.0f, 0.0f),
    XMFLOAT4(-0.345866f, -0.564379f, 0.0f, 0.0f),
    XMFLOAT4(-0.325663f, 0.64037f, 0.0f, 0.0f),
    XMFLOAT4(-0.182714f, 0.321329f, 0.0f, 0.0f),
    XMFLOAT4(-0.142613f, -0.0227363f, 0.0f, 0.0f),
    XMFLOAT4(-0.0564287f, -0.36729f, 0.0f, 0.0f),
    XMFLOAT4(-0.0185858f, 0.918882f, 0.0f, 0.0f),
    XMFLOAT4(0.0381787f, -0.728996f, 0.0f, 0.0f),
    XMFLOAT4(0.16599f, 0.093112f, 0.0f, 0.0f),
    XMFLOAT4(0.253639f, 0.719535f, 0.0f, 0.0f),
    XMFLOAT4(0.369549f, -0.655019f, 0.0f, 0.0f),
    XMFLOAT4(0.423627f, 0.429975f, 0.0f, 0.0f),
    XMFLOAT4(0.530747f, -0.364971f, 0.0f, 0.0f),
    XMFLOAT4(0.566027f, -0.940489f, 0.0f, 0.0f),
    XMFLOAT4(0.639332f, 0.0284127f, 0.0f, 0.0f),
    XMFLOAT4(0.652089f, 0.669668f, 0.0f, 0.0f),
    XMFLOAT4(0.773797f, 0.345012f, 0.0f, 0.0f),
    XMFLOAT4(0.968871f, 0.840449f, 0.0f, 0.0f),
    XMFLOAT4(0.991882f, -0.657338f, 0.0f, 0.0f),
};

const XMFLOAT4 Poisson::Poisson32[32] =
{
    XMFLOAT4(-0.975402f, -0.0711386f, 0.0f, 0.0f),
    XMFLOAT4(-0.920347f, -0.41142f, 0.0f, 0.0f),
    XMFLOAT4(-0.883908f, 0.217872f, 0.0f, 0.0f),
    XMFLOAT4(-0.884518f, 0.568041f, 0.0f, 0.0f),
    XMFLOAT4(-0.811945f, 0.90521f, 0.0f, 0.0f),
    XMFLOAT4(-0.792474f, -0.779962f, 0.0f, 0.0f),
    XMFLOAT4(-0.614856f, 0.386578f, 0.0f, 0.0f),
    XMFLOAT4(-0.580859f, -0.208777f, 0.0f, 0.0f),
    XMFLOAT4(-0.53795f, 0.716666f, 0.0f, 0.0f),
    XMFLOAT4(-0.515427f, 0.0899991f, 0.0f, 0.0f),
    XMFLOAT4(-0.454634f, -0.707938f, 0.0f, 0.0f),
    XMFLOAT4(-0.420942f, 0.991272f, 0.0f, 0.0f),
    XMFLOAT4(-0.261147f, 0.588488f, 0.0f, 0.0f),
    XMFLOAT4(-0.211219f, 0.114841f, 0.0f, 0.0f),
    XMFLOAT4(-0.146336f, -0.259194f, 0.0f, 0.0f),
    XMFLOAT4(-0.139439f, -0.888668f, 0.0f, 0.0f),
    XMFLOAT4(0.0116886f, 0.326395f, 0.0f, 0.0f),
    XMFLOAT4(0.0380566f, 0.625477f, 0.0f, 0.0f),
    XMFLOAT4(0.0625935f, -0.50853f, 0.0f, 0.0f),
    XMFLOAT4(0.125584f, 0.0469069f, 0.0f, 0.0f),
    XMFLOAT4(0.169469f, -0.997253f, 0.0f, 0.0f),
    XMFLOAT4(0.320597f, 0.291055f, 0.0f, 0.0f),
    XMFLOAT4(0.359172f, -0.633717f, 0.0f, 0.0f),
    XMFLOAT4(0.435713f, -0.250832f, 0.0f, 0.0f),
    XMFLOAT4(0.507797f, -0.916562f, 0.0f, 0.0f),
    XMFLOAT4(0.545763f, 0.730216f, 0.0f, 0.0f),
    XMFLOAT4(0.56859f, 0.11655f, 0.0f, 0.0f),
    XMFLOAT4(0.743156f, -0.505173f, 0.0f, 0.0f),
    XMFLOAT4(0.736442f, -0.189734f, 0.0f, 0.0f),
    XMFLOAT4(0.843562f, 0.357036f, 0.0f, 0.0f),
    XMFLOAT4(0.865413f, 0.763726f, 0.0f, 0.0f),
    XMFLOAT4(0.872005f, -0.927f, 0.0f, 0.0f),
};

const XMFLOAT4 Poisson::Poisson64[64] =
{
    XMFLOAT4(-0.934812f, 0.366741f, 0.0f, 0.0f),
    XMFLOAT4(-0.918943f, -0.0941496f, 0.0f, 0.0f),
    XMFLOAT4(-0.873226f, 0.62389f, 0.0f, 0.0f),
    XMFLOAT4(-0.8352f, 0.937803f, 0.0f, 0.0f),
    XMFLOAT4(-0.822138f, -0.281655f, 0.0f, 0.0f),
    XMFLOAT4(-0.812983f, 0.10416f, 0.0f, 0.0f),
    XMFLOAT4(-0.786126f, -0.767632f, 0.0f, 0.0f),
    XMFLOAT4(-0.739494f, -0.535813f, 0.0f, 0.0f),
    XMFLOAT4(-0.681692f, 0.284707f, 0.0f, 0.0f),
    XMFLOAT4(-0.61742f, -0.234535f, 0.0f, 0.0f),
    XMFLOAT4(-0.601184f, 0.562426f, 0.0f, 0.0f),
    XMFLOAT4(-0.607105f, 0.847591f, 0.0f, 0.0f),
    XMFLOAT4(-0.581835f, -0.00485244f, 0.0f, 0.0f),
    XMFLOAT4(-0.554247f, -0.771111f, 0.0f, 0.0f),
    XMFLOAT4(-0.483383f, -0.976928f, 0.0f, 0.0f),
    XMFLOAT4(-0.476669f, -0.395672f, 0.0f, 0.0f),
    XMFLOAT4(-0.439802f, 0.362407f, 0.0f, 0.0f),
    XMFLOAT4(-0.409772f, -0.175695f, 0.0f, 0.0f),
    XMFLOAT4(-0.367534f, 0.102451f, 0.0f, 0.0f),
    XMFLOAT4(-0.35313f, 0.58153f, 0.0f, 0.0f),
    XMFLOAT4(-0.341594f, -0.737541f, 0.0f, 0.0f),
    XMFLOAT4(-0.275979f, 0.981567f, 0.0f, 0.0f),
    XMFLOAT4(-0.230811f, 0.305094f, 0.0f, 0.0f),
    XMFLOAT4(-0.221656f, 0.751152f, 0.0f, 0.0f),
    XMFLOAT4(-0.214393f, -0.0592364f, 0.0f, 0.0f),
    XMFLOAT4(-0.204932f, -0.483566f, 0.0f, 0.0f),
    XMFLOAT4(-0.183569f, -0.266274f, 0.0f, 0.0f),
    XMFLOAT4(-0.123936f, -0.754448f, 0.0f, 0.0f),
    XMFLOAT4(-0.0859096f, 0.118625f, 0.0f, 0.0f),
    XMFLOAT4(-0.0610675f, 0.460555f, 0.0f, 0.0f),
    XMFLOAT4(-0.0234687f, -0.962523f, 0.0f, 0.0f),
    XMFLOAT4(-0.00485244f, -0.373394f, 0.0f, 0.0f),
    XMFLOAT4(0.0213324f, 0.760247f, 0.0f, 0.0f),
    XMFLOAT4(0.0359813f, -0.0834071f, 0.0f, 0.0f),
    XMFLOAT4(0.0877407f, -0.730766f, 0.0f, 0.0f),
    XMFLOAT4(0.14597f, 0.281045f, 0.0f, 0.0f),
    XMFLOAT4(0.18186f, -0.529649f, 0.0f, 0.0f),
    XMFLOAT4(0.188208f, -0.289529f, 0.0f, 0.0f),
    XMFLOAT4(0.212928f, 0.063509f, 0.0f, 0.0f),
    XMFLOAT4(0.23661f, 0.566027f, 0.0f, 0.0f),
    XMFLOAT4(0.266579f, 0.867061f, 0.0f, 0.0f),
    XMFLOAT4(0.320597f, -0.883358f, 0.0f, 0.0f),
    XMFLOAT4(0.353557f, 0.322733f, 0.0f, 0.0f),
    XMFLOAT4(0.404157f, -0.651479f, 0.0f, 0.0f),
    XMFLOAT4(0.410443f, -0.413068f, 0.0f, 0.0f),
    XMFLOAT4(0.413556f, 0.123325f, 0.0f, 0.0f),
    XMFLOAT4(0.46556f, -0.176183f, 0.0f, 0.0f),
    XMFLOAT4(0.49266f, 0.55388f, 0.0f, 0.0f),
    XMFLOAT4(0.506333f, 0.876888f, 0.0f, 0.0f),
    XMFLOAT4(0.535875f, -0.885556f, 0.0f, 0.0f),
    XMFLOAT4(0.615894f, 0.0703452f, 0.0f, 0.0f),
    XMFLOAT4(0.637135f, -0.637623f, 0.0f, 0.0f),
    XMFLOAT4(0.677236f, -0.174291f, 0.0f, 0.0f),
    XMFLOAT4(0.67626f, 0.7116f, 0.0f, 0.0f),
    XMFLOAT4(0.686331f, -0.389935f, 0.0f, 0.0f),
    XMFLOAT4(0.691031f, 0.330729f, 0.0f, 0.0f),
    XMFLOAT4(0.715629f, 0.999939f, 0.0f, 0.0f),
    XMFLOAT4(0.8493f, -0.0485549f, 0.0f, 0.0f),
    XMFLOAT4(0.863582f, -0.85229f, 0.0f, 0.0f),
    XMFLOAT4(0.890622f, 0.850581f, 0.0f, 0.0f),
    XMFLOAT4(0.898068f, 0.633778f, 0.0f, 0.0f),
    XMFLOAT4(0.92053f, -0.355693f, 0.0f, 0.0f),
    XMFLOAT4(0.933348f, -0.62981f, 0.0f, 0.0f),
    XMFLOAT4(0.95294f, 0.156896f, 0.0f, 0.0f),
};

const XMFLOAT4* Poisson::GetDistribution(UINT idx)
{
    UINT size = DISTRIBUTION_SIZES[idx];
    if (size == 12)
    {
        return Poisson12;
    }
    else if (size == 16)
    {
        return Poisson16;
    }
    else if (size == 25)
    {
        return Poisson25;
    }
    else if (size == 32)
    {
        return Poisson32;
    }
    else if (size == 64)
    {
        return Poisson64;
    }
    else
    {
        return NULL;
    }
}

UINT Poisson::GetDistributionSize(UINT idx)
{
    return DISTRIBUTION_SIZES[idx];
}