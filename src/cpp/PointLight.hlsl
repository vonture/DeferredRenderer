cbuffer cbCameraProperties : register(b0)
{
	float4x4 InverseViewProjection	: packoffset(c0);
	float4   CameraPosition			: packoffset(c4);
}

cbuffer cbModelProperties : register(b1)
{
    float4x4 World					: packoffset(c0);
    float4x4 WorldViewProjection	: packoffset(c4);
}

cbuffer cbLightProperties : register(b2)
{
	float3 LightPosition	: packoffset(c0);
	float  LightRadius		: packoffset(c0.w);
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
	float4 vPositionCS2	: TEXCOORD0;
};

VS_Out_PointLight VS_PointLight(VS_In_PointLight input)
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

float2 GetScreenTexCoord(float2 vPositionCS)
{
	return (float2(vPositionCS.x, -vPositionCS.y) + 1.0f) * 0.5f;
}

float4 PS_PointLightCommon(VS_Out_PointLight input, float4 vPositionWS, float2 vTexCoord)
{
	float4 vColorData = RT0.Sample(LinearSampler, vTexCoord);
    float4 vNormalData = RT1.Sample(LinearSampler, vTexCoord);

	float fSpecularIntensity = vColorData.a;
	float fSpecularPower = vNormalData.a;	

    float3 vLightDir = LightPosition - vPositionWS.xyz;

	float fAttenuation = saturate(1.0f - (length(vLightDir) / LightRadius));
	fAttenuation = fAttenuation * fAttenuation;

	float3 N = vNormalData.xyz;
    float3 L = normalize(vLightDir);
	float3 V = normalize(CameraPosition - vPositionWS).xyz;

	float3 R = normalize(V - 2 * dot(N, V) * N);

    float fDiffuseTerm = saturate(dot(N, L));
	float fSpecularTerm = fSpecularIntensity * pow(saturate(dot(R, L)), fSpecularPower);

	return fAttenuation * LightIntensity * float4(fDiffuseTerm * LightColor, fSpecularTerm);
}

float4 PS_PointLightUnshadowed(VS_Out_PointLight input) : SV_TARGET0
{
	input.vPositionCS2.xyz = input.vPositionCS2.xyz / input.vPositionCS2.www;
	input.vPositionCS2.w = 1.0f;

	float2 vScreenCoord = GetScreenTexCoord(input.vPositionCS2.xy);

	float fDepth = RT3.Sample(LinearSampler, vScreenCoord).r;
	float4 vPositionWS = GetPositionWS(input.vPositionCS2.xy, fDepth);

	return PS_PointLightCommon(input, vPositionWS, vScreenCoord);
}

float4 PS_PointLightShadowed(VS_Out_PointLight input) : SV_TARGET0
{
	input.vPositionCS2.xyz = input.vPositionCS2.xyz / input.vPositionCS2.www;
	input.vPositionCS2.w = 1.0f;

	float2 vScreenCoord = GetScreenTexCoord(input.vPositionCS2.xy);

	float fDepth = RT3.Sample(LinearSampler, vScreenCoord).r;
	float4 vPositionWS = GetPositionWS(input.vPositionCS2.xy, fDepth);

	return PS_PointLightCommon(input, vPositionWS, vScreenCoord);
}