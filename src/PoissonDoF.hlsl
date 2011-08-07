#ifndef DOF_SAMPLE_COUNT
#define DOF_SAMPLE_COUNT 16
#endif

cbuffer cbDoFProperties : register(cb0)
{
	float CameraNearClip			: packoffset(c0.x);
	float CameraFarClip				: packoffset(c0.y);
	float FocalDistance				: packoffset(c0.z);
	float FocalFalloffNear			: packoffset(c0.w);
	float FocalFalloffFar			: packoffset(c1.x);
	float CircleOfConfusionScale	: packoffset(c1.y);
	float2 Padding					: packoffset(c1.z);
}

cbuffer cbSampleDirections : register(cb0)
{
	float4 SampleDirections[DOF_SAMPLE_COUNT]	: packoffset(c0.x);
}

Texture2D DepthTexture : register(t0);
Texture2D SceneTexture : register(t1);

SamplerState PointSampler  : register(s0);
SamplerState LinearSampler : register(s1);

struct PS_In_Quad
{
    float4 vPosition	: SV_POSITION;
    float2 vTexCoord	: TEXCOORD0;
	float2 vPosition2	: TEXCOORD1;
};

float GetLinearDepth(float fNonLinearDepth)
{
	float fPercFar = CameraFarClip / (CameraFarClip - CameraNearClip);
	return (-CameraNearClip * fPercFar) / (fNonLinearDepth - fPercFar);
}

float GetCoCSize(float fLinearDepth)
{
	float fDistFromFocalDepth = fLinearDepth - FocalDistance;
	
	float fBlurAmmount = max(abs(fDistFromFocalDepth) - FocalFalloffNear, 0.0f) / 
						 (FocalFalloffFar - FocalFalloffNear);

	return smoothstep(0.0f, 1.0f, fBlurAmmount) * sign(fDistFromFocalDepth);
}

float4 PS_PoissonDoF(PS_In_Quad input) : SV_TARGET0
{
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}