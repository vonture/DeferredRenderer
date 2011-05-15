cbuffer cbModelProperties : register(b0)
{
    float4x4 World					: packoffset(c0);
    float4x4 WorldViewProjection	: packoffset(c4);
}

cbuffer cbCameraProperties : register(b1)
{
	float4x4 InverseViewProjection	: packoffset(c0);
	float4   CameraPosition			: packoffset(c4);
}

cbuffer cbLightProperties : register(b2)
{
	float3 LightPosition	: packoffset(c0);
	float  LightRadius		: packoffset(c0.w)	
	float3 LightColor		: packoffset(c1);	
	float  LightIntensity	: packoffset(c1.w);	
}

Texture2D RT0 : register(t0);
Texture2D RT1 : register(t1);
Texture2D RT2 : register(t2);
Texture2D RT3 : register(t3);

SamplerState LinearSampler	: register(s0);
SamplerState ShadowSampler	: register(s1);

struct VS_In_PointLight
{
	float4 vPositionOS	: POSITION;
};

struct VS_Out_PointLight
{
	float4 vPositionCS	: SV_POSITION;
	float2 vPositionCS2	: TEXCOORD0;
};

VS_Out_PointLight VS_PointLight(VS_In_Depth input)
{
    VS_Out_PointLight output;

    output.vPositionCS = mul(input.vPositionOS, WorldViewProjection);
	output.vPositionCS2 = output.vPositionCS;

    return output;
}

float4 GetPositionWS(float2 vPositionCS, float fDepth)
{
	float4 vPositionWS = mul(float4(vPositionCS, fDepth, 1.0f), InverseViewProjection);
	vPositionWS.xyz = vPositionWS.xyz / vPositionWS.www;
	vPositionWS.w = 1.0f;

	return vPositionWS;
}

float4 PS_PointLightCommon(VS_Out_PointLight input, float4 vPositionWS)
{
	float4 vColorData = RT0.Sample(LinearSampler, input.vTexCoord);
    float4 vNormalData = RT1.Sample(LinearSampler, input.vTexCoord);

	float fSpecularIntensity = vColorData.a;
	float fSpecularPower = vNormalData.a * 255.0f;	

    float3 vLightDir = LightPosition - vPositionWS.xyz;

	float fAttenuation = saturate(1.0f - (length(vLightDir) / LightRadius));

	float3 N = vNormalData.xyz * 2.0f - 1.0f;
    float3 L = normalize(vLightDir);
	float3 V = normalize(CameraPosition - vPositionWS);

	float3 R = normalize(V - 2 * dot(N, V) * N);

    float fDiffuseTerm = saturate(dot(N, L));
	float fSpecularTerm = fSpecularIntensity * pow(saturate(dot(R, L)), fSpecularPower);

	return fAttenuation * LightIntensity * float4(fDiffuseTerm * LightColor, fSpecularTerm);
}

float4 PS_PointLightUnshadowed(VS_Out_PointLight input) : SV_TARGET0
{
	float fDepth = RT3.Sample(LinearSampler, input.vTexCoord).r;
	float4 vPositionWS = GetPositionWS(input.vPosition, fDepth);

	return PS_PointLightCommon(input, vPositionWS);
}

float4 PS_PointLightShadowed(VS_Out_PointLight input) : SV_TARGET0
{
	float fDepth = RT3.Sample(LinearSampler, input.vTexCoord).r;
	float4 vPositionWS = GetPositionWS(input.vPosition, fDepth);

	return PS_PointLightCommon(input, vPositionWS);
}