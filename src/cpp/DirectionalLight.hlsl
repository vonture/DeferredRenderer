static const int CascadeCount = 4;
static const int SqrtCascadeCount = 2;
static const int PCFRadius = 2;

cbuffer cbCameraProperties : register(b0)
{
	float4x4 InverseViewProjection	: packoffset(c0);
	float4   CameraPosition			: packoffset(c4);
}

cbuffer cbLightProperties : register(b1)
{
	float3 LightDirection	: packoffset(c0);
	float  LightIntensity	: packoffset(c0.w);
	float4 LightColor		: packoffset(c1);	
}

cbuffer cbShadowProperties : register(b2)
{
	float2 CameraClips;
	float2 ShadowMapSize;
	float4 CascadeSplits;
	float4x4 ShadowMatrices[CascadeCount];	
	float4x4 ShadowTexCoordTransforms[CascadeCount];
}

Texture2D RT0 : register(t0);
Texture2D RT1 : register(t1);
Texture2D RT2 : register(t2);
Texture2D RT3 : register(t3);

Texture2D ShadowMap : register(t5);

SamplerState SceneSampler	: register(s0);
SamplerComparisonState ShadowSampler	: register(s1);

struct PS_In_DirectionalLight
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

float4 PS_DirectionalLightCommon(PS_In_DirectionalLight input, float4 vPositionWS)
{
	float4 vColorData = RT0.Sample(SceneSampler, input.vTexCoord);
    float4 vNormalData = RT1.Sample(SceneSampler, input.vTexCoord);
    
	float fSpecularIntensity = vColorData.a;
	float fSpecularPower = vNormalData.a;	
		    
	float3 N = vNormalData.xyz;
    float3 L = normalize(LightDirection);
	float3 V = normalize(CameraPosition.xyz - vPositionWS.xyz);

    float3 R = normalize(V - 2 * dot(N, V) * N);

	float fDiffuseTerm = saturate(dot(N, L));
	float fSpecularTerm = fSpecularIntensity * pow(saturate(dot(R, L)), fSpecularPower);
	
    return LightIntensity * float4(fDiffuseTerm * LightColor.rgb, fSpecularTerm);
}

float4 PS_DirectionalLightUnshadowed(PS_In_DirectionalLight input) : SV_TARGET0
{
	float fDepth = RT3.Sample(SceneSampler, input.vTexCoord).r;
	float4 vPositionWS = GetPositionWS(input.vPosition2, fDepth);

	return PS_DirectionalLightCommon(input, vPositionWS);
};

float SampleShadowCascade(in float4 positionWS, in uint cascadeIdx)
{
	float4x4 shadowMatrix = mul(ShadowMatrices[cascadeIdx], ShadowTexCoordTransforms[cascadeIdx]);
	float4 shadowPosition = mul(positionWS, shadowMatrix);
	float2 shadowTexCoord = shadowPosition.xy / shadowPosition.w;
	float shadowDepth = shadowPosition.z / shadowPosition.w;

	// Edge tap smoothing
	const int NumSamples = (PCFRadius * 2 + 1) * (PCFRadius * 2 + 1);

	float2 fracs = frac(shadowTexCoord.xy * ShadowMapSize);
	float leftEdge = 1.0f - fracs.x;
	float rightEdge = fracs.x;
	float topEdge = 1.0f - fracs.y;
	float bottomEdge = fracs.y;

	float shadowVisibility = 0.0f;

	[unroll(NumSamples)]
	for (int y = -PCFRadius; y <= PCFRadius; y++)
	{
		[unroll(NumSamples)]
		for (int x = -PCFRadius; x <= PCFRadius; x++)
		{
			int2 offset = int2(x, y);
			float sample = ShadowMap.SampleCmp(ShadowSampler, shadowTexCoord, shadowDepth, offset).x;

			float xWeight = 1;
			float yWeight = 1;

			if(x == -PCFRadius)
			{
				xWeight = leftEdge;
			}
			else if(x == PCFRadius)
			{
				xWeight = rightEdge;
			}

			if(y == -PCFRadius)
			{
				yWeight = topEdge;
			}
			else if(y == PCFRadius)
			{
				yWeight = bottomEdge;
			}

			shadowVisibility += sample * xWeight * yWeight;
		}
	}

	shadowVisibility /= NumSamples;
	shadowVisibility *= 1.5f;

	return shadowVisibility;
}

float4 PS_DirectionalLightShadowed(PS_In_DirectionalLight input) : SV_TARGET0
{
	float fDepth = RT3.Sample(SceneSampler, input.vTexCoord).r;
	float4 vPositionWS = GetPositionWS(input.vPosition2, fDepth);
		
	// Determine how much padding is required in pixels on the shadow map so that PCF can be 
	// done without moving into another cascade and causing artifacts
	float2 vPaddingRequired = (1.0f / ShadowMapSize) * SqrtCascadeCount * PCFRadius;

	// Find the correct cascade by calculating the tex coord in [0,1] space of the shadow
	// map and checking that it is within the usuable bounds.  Stops when it finds the closest
	// cascade
	uint iCascadeIdx = 0;
	uint iCascadeFound = 0;
	[unroll]
	for (int i = 0; !iCascadeFound && i < CascadeCount; i++)
	{
		float4 shadowPosition = mul(vPositionWS, ShadowMatrices[i]);
		float2 vShadowTexCoord = shadowPosition.xy / shadowPosition.w;

		if (vShadowTexCoord.x > vPaddingRequired.x && 
			vShadowTexCoord.y > vPaddingRequired.y && 
			vShadowTexCoord.x < (1.0f - vPaddingRequired.x) && 
			vShadowTexCoord.y < (1.0f - vPaddingRequired.x))
		{
			iCascadeIdx = i;
			iCascadeFound = 1;
		}
	}

	float fShadow = SampleShadowCascade(vPositionWS, iCascadeIdx);

	return fShadow * PS_DirectionalLightCommon(input, vPositionWS);	
};