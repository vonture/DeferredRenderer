#ifndef SUN_ENABLED
#define SUN_ENABLED 0
#endif

#define PI_OVER_TWO 1.57079633f

#ifndef UP
#define UP float3(0.0f, 1.0f, 0.0f)
#endif

#ifndef A
#define A -1.0f
#endif

#ifndef B
#define B -0.32f
#endif

#ifndef C
#define C 10.0f
#endif

#ifndef D
#define D -3.0f
#endif

#ifndef E
#define E 0.45f
#endif

cbuffer cbSkyProperties : register(cb0)
{
	float SunWidth					: packoffset(c0.x);
	float SunIntensity				: packoffset(c0.y);
	float3 SkyColor					: packoffset(c1.x);
	float3 SunColor					: packoffset(c2.x);
	float3 SunDirection				: packoffset(c3.x);	
	float3 CameraPosition			: packoffset(c4.x);
	float4x4 InverseViewProjection	: packoffset(c5.x);
}

struct PS_In_Quad
{
    float4 vPosition	: SV_POSITION;
    float2 vTexCoord	: TEXCOORD0;
	float2 vPosition2	: TEXCOORD1;
};

float4 GetPositionWS(float2 vPositionCS, float fDepth)
{
	float4 vPositionWS = mul(float4(vPositionCS.x, vPositionCS.y, fDepth, 1.0f), InverseViewProjection);
	vPositionWS.xyz = vPositionWS.xyz / vPositionWS.www;
	vPositionWS.w = 1.0f;

	return vPositionWS;
}

float AngleBetween(float3 dirA, float3 dirB)
{
	return acos(dot(dirA, dirB));
}

//-------------------------------------------------------------------------------------------------
// Uses the CIE Clear Sky model to compute a color for a pixel, given a direction + sun direction
//-------------------------------------------------------------------------------------------------
float Phi(float theta)
{
	return 1.0f + (A * exp(B / cos(theta)));
}

float F(float gamma)
{
	float cosGamma = cos(gamma);
	return 1.0f + (C * (exp(D * gamma) - exp(D * PI_OVER_TWO))) + (E * cosGamma * cosGamma);
}

float3 CIEClearSky(float3 dir, float3 sunDir)
{
	float3 skyDir = float3(dir.x, abs(dir.y), dir.z);
	float gamma = AngleBetween(skyDir, sunDir);
	float S = AngleBetween(sunDir, UP);
	float theta = AngleBetween(skyDir, UP);

	float sunContribution = F(gamma) / F(S);
	float zenithContribution = Phi(theta) / Phi(0.0f);
		
	// Slightly deviates from the model so that the color of the sun lights the sky around it
	float3 color = max((sunContribution * SunColor) + (zenithContribution * SkyColor), 0.0f);

#if SUN_ENABLED
	float sunGamma = AngleBetween(dir, sunDir);
	return lerp(SunColor * SunIntensity, color, saturate(abs(sunGamma) / SunWidth));
#else
	return color;
#endif
}

float4 PS_Sky(PS_In_Quad input) : SV_TARGET0
{
	float3 vSkyDirection = normalize(GetPositionWS(input.vPosition2, 1.0f).xyz - CameraPosition);
	float3 CIESkyColor = CIEClearSky(vSkyDirection, SunDirection);

	return float4(CIESkyColor, 1.0f);
}