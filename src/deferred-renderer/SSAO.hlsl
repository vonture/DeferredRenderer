#ifndef SSAO_SAMPLE_COUNT
#define SSAO_SAMPLE_COUNT 16
#endif

#ifndef SSAO_HALF_RES
#define SSAO_HALF_RES 1
#endif

#ifndef BLUR_RADIUS
#define BLUR_RADIUS 3
#endif

#ifndef RAND_TEX_SIZE
#define RAND_TEX_SIZE 32
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
	float SamplePower				: packoffset(c9.y);
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
	// Texture0 = Normals
	// Texture1 = Depth
	// Texture2 = Random noise
	float3 vNormal = Texture0.SampleLevel(PointSampler, input.vTexCoord, 0).xyz;
	
	float fDepth = Texture1.SampleLevel(PointSampler, input.vTexCoord, 0).x;
	float4 vPositionWS = GetPositionWS(input.vPosition2, fDepth);

	// Sample the random texture so that this location will always yeild the same
	// random direction (so that there is no flickering)
	float3 vRandomDirection = Texture2.SampleLevel(PointSampler, frac(input.vTexCoord * RAND_TEX_SIZE), 0).xyz;
	
	float fAOSum = 0.0f;
	[unroll]
	for (int i = 0; i < SSAO_SAMPLE_COUNT; i++)
	{
		// Create the ray
		float3 ray = reflect(SampleDirections[i].xyz, vRandomDirection) * SampleRadius;
		
		// Invert the ray if it points into the surface
		ray = ray * sign(dot(ray, vNormal));
		
		// Calculate the position to be sampled
		float4 vSamplePositionWS = float4(vPositionWS.xyz + ray, 1.0f);

		// Determine the screen space location of the sample
		float4 vSamplePositionCS = mul(vSamplePositionWS, ViewProjection);
		vSamplePositionCS.xyz = vSamplePositionCS.xyz / vSamplePositionCS.w;
		vSamplePositionCS.w = 1.0f;
		
		// Transform the camera space position to a texture coordinate
		float2 vSampleTexCoord = (0.5f * vSamplePositionCS.xy) + float2(0.5f, 0.5f);
		vSampleTexCoord.y = 1.0f - vSampleTexCoord.y;

		// Sample the depth of the new location
		float fSampleDepth = Texture1.SampleLevel(PointSampler, vSampleTexCoord, 0).x;
		
		float fSampleLinearDepth = GetLinearDepth(fSampleDepth, CameraNearClip, CameraFarClip);
		float fRayLinearDepth = GetLinearDepth(vSamplePositionCS.z, CameraNearClip, CameraFarClip);

		// Calculate the occlusion
		float fOcclusion = 1.0f;

		float fRaySampleDist = fRayLinearDepth - fSampleLinearDepth;
		if (fRaySampleDist < SampleRadius && fRaySampleDist > 0.0f && fSampleDepth < 1.0f)
		{
			fOcclusion = pow(abs(fRaySampleDist / SampleRadius), SamplePower);
		}

		fAOSum += fOcclusion;
	}

	float fAO = fAOSum / SSAO_SAMPLE_COUNT;

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
float Blur(float2 texCoord, int2 direction)
{
#if SSAO_HALF_RES
	texCoord = texCoord * 0.5f;
#endif

	float weightMid = CalcGaussianWeight(0);
	float sampleMid = Texture0.SampleLevel(PointSampler, texCoord, 0).x;
    float value = sampleMid * weightMid;

	[unroll]
    for (int i = 1; i <= BLUR_RADIUS; i++)
    {
		float weight = CalcGaussianWeight(i);

		float sampleDown = Texture0.SampleLevel(PointSampler, texCoord, 0, direction * i).x;
		float sampleUp = Texture0.SampleLevel(PointSampler, texCoord, 0, direction * i).x;

		value = value + (sampleDown + sampleUp) * weight;
    }

    return value;
}

// Horizontal gaussian blur
float4 PS_BlurHorizontal(PS_In_Quad input) : SV_TARGET0
{
    return float4(Blur(input.vTexCoord, int2(2, 0)), 0.0f, 0.0f, 1.0f);
}

// Vertical gaussian blur
float4 PS_BlurVertical(PS_In_Quad input) : SV_TARGET0
{
	return float4(Blur(input.vTexCoord, int2(0, 2)), 0.0f, 0.0f, 1.0f);
}