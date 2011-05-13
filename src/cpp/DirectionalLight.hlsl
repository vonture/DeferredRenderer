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
	float4 CameraClips;
	float4 CascadeSplits;
	float4x4 ShadowMatrices[4];	
}

static const int SqrtCascadeCount = 2;

Texture2D RT0 : register(t0);
Texture2D RT1 : register(t1);
Texture2D RT2 : register(t2);
Texture2D RT3 : register(t3);

Texture2D ShadowMap : register(t5);

SamplerState LinearSampler				: register(s0);
SamplerComparisonState ShadowSampler	: register(s1);

struct PS_In_DirectionalLight
{
    float4 vPosition : SV_POSITION;              
    float2 vTexCoord : TEXCOORD0;
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
	float4 vColorData = RT0.Sample(LinearSampler, input.vTexCoord);
    float4 vNormalData = RT1.Sample(LinearSampler, input.vTexCoord);
    
	float fSpecularIntensity = vColorData.a;
	float fSpecularPower = vNormalData.a * 255.0f;	
		    
	float3 N = vNormalData.xyz;
    float3 L = normalize(LightDirection);
	float3 V = normalize(CameraPosition - vPositionWS);

    float3 R = normalize(V - 2 * dot(N, V) * N);

	float fDiffuseTerm = saturate(dot(N, L));
	float fSpecularTerm = fSpecularIntensity * pow(saturate(dot(R, L)), fSpecularPower);
	
    return LightIntensity * float4(fDiffuseTerm * LightColor.rgb, fSpecularTerm);
}

float4 PS_DirectionalLightUnshadowed(PS_In_DirectionalLight input) : SV_TARGET0
{
	float fDepth = RT3.Sample(LinearSampler, input.vTexCoord).r;
	float4 vPositionWS = GetPositionWS(input.vPosition, fDepth);

	return PS_DirectionalLightCommon(input, vPositionWS);
};

float4 PS_DirectionalLightShadowed(PS_In_DirectionalLight input) : SV_TARGET0
{
	float fDepth = RT3.Sample(LinearSampler, input.vTexCoord).r;
	float4 vPositionWS = GetPositionWS(input.vPosition.xy, fDepth);
	
	float fPercFar = CameraClips.y / (CameraClips.y - CameraClips.x);
	float fSceneZ = ( -CameraClips.x * fPercFar ) / ( fDepth - fPercFar);	

	uint iCascadeIdx = 0;
	[unroll]
	for (uint i = 0; i < SqrtCascadeCount * SqrtCascadeCount; i++)
	{
		[flatten]
		if(fSceneZ > CascadeSplits[i])
		{
			iCascadeIdx = i + 1;
		}
	}

	float4x4 mShadowMatrix = ShadowMatrices[iCascadeIdx];

	float4 vLightPosition = mul(vPositionWS, mShadowMatrix);
	
	float fLightDepth = vLightPosition.z / vLightPosition.w;
	float2 vShadowTexCoord = vLightPosition.xy / vLightPosition.w;

	//float shadowSample = ShadowMap.SampleCmp(ShadowSampler, vShadowTexCoord, fLightDepth).x;
	float shadowSample = ShadowMap.Sample(LinearSampler, vShadowTexCoord).x;
	
	//return float4((input.vPosition.xyz + 1.0f) * 0.5f, 1.0f);
	//return float4(fDepth,fDepth,fDepth, 1.0f);
	//return float4(iCascadeIdx / 4.0f, 0.0f, 0.0f, 1.0f);
	return float4(shadowSample, 0.0f, 0.0f, 1.0f);
	//return float4(ShadowMap.Sample(LinearSampler, input.vTexCoord).rrr, 1.0f);
	//return float4(vShadowTexCoord ,0.0f, 1.0f);
	//return float4(fLightDepth, fLightDepth,fLightDepth, 1.0f);
	//return shadowSample * PS_DirectionalLightCommon(input, vPositionWS);
};