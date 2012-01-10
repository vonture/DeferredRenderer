#ifndef SSAO_SAMPLE_COUNT
#define SSAO_SAMPLE_COUNT 16
#endif

#ifndef SSAO_HALF_RES
#define SSAO_HALF_RES 1
#endif

#ifndef BLUR_RADIUS
#define BLUR_RADIUS 4
#endif

#ifndef RAND_TEX_SIZE
#define RAND_TEX_SIZE 32
#endif

#ifndef DEPTH_BIAS
#define DEPTH_BIAS 0.01f
#endif

#ifndef BLUR_EDGE_THRESHOLD
#define BLUR_EDGE_THRESHOLD 0.85f
#endif

cbuffer cbSSAOProperties : register(cb0)
{
	float4x4 ViewProjection			: packoffset(c0.x);
	float4x4 InverseViewProjection	: packoffset(c4.x);
	float SampleRadius				: packoffset(c8.x);
	float BlurSigma					: packoffset(c8.y);
	float GaussianNumerator			: packoffset(c8.z);
	float CameraNearClip			: packoffset(c8.w);
	float CameraFarClip				: packoffset(c9.x);
	float DepthThreshold			: packoffset(c9.y);
	float2 InverseSceneSize			: packoffset(c9.z);
}

cbuffer cbSSAOSampleDirections : register(cb1)
{	
	float4 SampleDirections[SSAO_SAMPLE_COUNT]	: packoffset(c0.x);
}

Texture2D Texture0 : register(t0);
Texture2D Texture1 : register(t1);
Texture2D Texture2 : register(t2);

SamplerState PointSampler  : register(s0);
SamplerState LinearSampler : register(s1);

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

float GetLinearDepth(float nonLinearDepth, float nearClip, float farClip)
{
	float fPercFar = farClip / (farClip - nearClip);
	return ( -nearClip * fPercFar ) / ( nonLinearDepth - fPercFar);
}

float4 PS_SSAO(PS_In_Quad input) : SV_TARGET0
{
	// Texture0 = Depth
	// Texture1 = Random noise
	float fCenterDepth = GetLinearDepth(Texture0.SampleLevel(PointSampler, input.vTexCoord, 0).x, CameraNearClip, CameraFarClip);
	
	// Sample the random texture so that this location will always yeild the same
	// random direction (so that there is no flickering)
	float fRandomRotation = Texture1.SampleLevel(PointSampler, frac(input.vTexCoord * RAND_TEX_SIZE), 0).x;
	
	float fUnoccludedSamples = 0;

	[unroll]
	for (int i = 0; i < SSAO_SAMPLE_COUNT; i++)
	{
		float fSampleAngle = SampleDirections[i].x + fRandomRotation;

		float2 vRayDir = float2(cos(fSampleAngle), sin(fSampleAngle)) * InverseSceneSize * SampleDirections[i].y * SampleRadius;
		
		float2 vRayTexCoord = input.vTexCoord + vRayDir;
		float fRayDepth = GetLinearDepth(Texture0.SampleLevel(PointSampler, vRayTexCoord, 0).x, CameraNearClip, CameraFarClip);
		
		if (abs(fRayDepth - fCenterDepth) > DepthThreshold || (fRayDepth + DEPTH_BIAS) >= fCenterDepth)
		{
			fUnoccludedSamples++;
			//fUnoccludedSamples += (1.0f - min(abs(fRayDepth - fCenterDepth) / DepthThreshold, 1.0f));
		}
	}

	float fAO = saturate((fUnoccludedSamples / SSAO_SAMPLE_COUNT) * 2.0f);

	return float4(fAO, 0.0f, 0.0f, 1.0f);
}

float4 PS_SSAO_Composite(PS_In_Quad input) : SV_TARGET0
{
	float3 vSceneColor = Texture0.SampleLevel(PointSampler, input.vTexCoord, 0).rgb;
	
#if SSAO_HALF_RES
	float fAO = Texture1.SampleLevel(LinearSampler, input.vTexCoord * 0.5f, 0).x;
#else
	float fAO = Texture1.SampleLevel(PointSampler, input.vTexCoord, 0).x;
#endif	

	return float4(fAO * vSceneColor, 1.0f);
}

// Calculates the gaussian blur weight for a given distance and sigmas
float CalcGaussianWeight(int sampleDist)
{
	return (GaussianNumerator * exp(-(sampleDist * sampleDist) / (2 * BlurSigma * BlurSigma)));
}

// Performs a gaussian blur in one direction
float Blur(float2 vTexCoord, float2 vDirection)
{
	// Texture0 = AO
	// Texture1 = Normals

	float2 vNormStep = InverseSceneSize;
#if SSAO_HALF_RES
	float2 vAOTexCoord = (vTexCoord * 0.5f) + (InverseSceneSize * 0.25f);
	float2 vAOStep = (InverseSceneSize * 0.5f) * vDirection;
#else
	float2 vAOTexCoord = vTexCoord + (InverseSceneSize * 0.5f);
	float2 vAOStep = InverseSceneSize * vDirection;
#endif

    float fTotalValue = 0.0f;
	float fTotalWeight = 1.0f;

	float3 fCenterNormal = Texture1.SampleLevel(PointSampler, vTexCoord, 0).xyz;

	[unroll]
    for (int i = -BLUR_RADIUS; i <= BLUR_RADIUS; i++)
    {
		float fWeight = CalcGaussianWeight(i);
		float3 fNormalSample = Texture1.SampleLevel(PointSampler, vTexCoord + (vNormStep * i), 0).xyz;

		if (dot(fNormalSample, fCenterNormal) > BLUR_EDGE_THRESHOLD)
		{
			float fAOSample = Texture0.SampleLevel(LinearSampler, vAOTexCoord + (vAOStep * i), 0).x;
			fTotalValue += fAOSample * fWeight;
		}
		else
		{
			fTotalWeight -= fWeight;
		}
    }

    return fTotalValue / fTotalWeight;
}

// Horizontal gaussian blur
float4 PS_BlurHorizontal(PS_In_Quad input) : SV_TARGET0
{
    return float4(Blur(input.vTexCoord, float2(2.0f, 0.0f)), 0.0f, 0.0f, 1.0f);
}

// Vertical gaussian blur
float4 PS_BlurVertical(PS_In_Quad input) : SV_TARGET0
{
	return float4(Blur(input.vTexCoord, float2(0.0f, 2.0f)), 0.0f, 0.0f, 1.0f);
}
