#define GREY float3(0.212671f, 0.715160f, 0.072169f)
#define EPSILON 0.0001f

#ifndef ALPHA_CUTOUT_ENABLED
#define ALPHA_CUTOUT_ENABLED 1
#endif

#ifndef DIFFUSE_MAPPED
#define DIFFUSE_MAPPED 1
#endif

#ifndef NORMAL_MAPPED
#define NORMAL_MAPPED 1
#endif

#ifndef SPECULAR_MAPPED
#define SPECULAR_MAPPED 0
#endif

// Vertex shader buffer
cbuffer cbModelProperties : register(c0)
{
    float4x4 World					: packoffset(c0.x);
    float4x4 WorldViewProjection	: packoffset(c4.x);
}

// Pixel shader buffers
cbuffer cbMaterialProperties : register(c0)
{
	float3 AmbientColor		: packoffset(c0);
    float3 DiffuseColor		: packoffset(c1);
    float3 EmissiveColor	: packoffset(c2);    
	float3 SpecularColor	: packoffset(c3);
	float SpecularPower		: packoffset(c4.x);
    float Alpha				: packoffset(c4.y);
	float2 Padding2			: packoffset(c4.z);
};

cbuffer cbAlphaCutoutProperties : register(c1)
{
	float AlphaThreshold	: packoffset(c0.x);
	float3 Padding1			: packoffset(c0.y);
};

Texture2D DiffuseMap	: register(t0);
Texture2D NormalMap		: register(t1);
Texture2D SpecularMap	: register(t2);

SamplerState Sampler	: register(s0);

struct VS_In_Mesh
{
	float4 vPositionOS	: POSITION;
	float3 vNormalOS	: NORMAL;
	float2 vTexCoord	: TEXCOORD;
	float3 vTangentOS	: TANGENT;
	float3 vBinormalOS	: BINORMAL;
};

struct VS_Out_Mesh
{
	float4 vPositionCS	: SV_POSITION;
	float2 vTexCoord	: TEXCOORD;
	float3 vNormalWS	: NORMALWS;
	float3 vTangentWS	: TANGENTWS;
	float3 vBinormalWS	: BINORMALWS;
};

struct PS_Out_Mesh
{    
	float4 RT0 : SV_TARGET0;
	float4 RT1 : SV_TARGET1;
	float4 RT2 : SV_TARGET2;
};

VS_Out_Mesh VS_Mesh(VS_In_Mesh input)
{
    VS_Out_Mesh output;

    output.vPositionCS = mul(input.vPositionOS, WorldViewProjection);
	output.vNormalWS = mul(input.vNormalOS, (float3x3)World);
	output.vTangentWS = mul(input.vTangentOS, (float3x3)World);
	output.vBinormalWS = mul(input.vBinormalOS, (float3x3)World);
	output.vTexCoord = input.vTexCoord;

    return output;
}

PS_Out_Mesh PS_Mesh(VS_Out_Mesh input)
{
	PS_Out_Mesh output;

#if DIFFUSE_MAPPED

	#if ALPHA_CUTOUT_ENABLED
		float fAlphaNoMip = DiffuseMap.SampleLevel(Sampler, input.vTexCoord, 0).a;
		clip(fAlphaNoMip - AlphaThreshold);
	#endif

	float4 vDiffuse = DiffuseMap.Sample(Sampler, input.vTexCoord);
#else
	float4 vDiffuse = float4(DiffuseColor, Alpha);
#endif
		
#if NORMAL_MAPPED
	float3 vNormalWS = normalize(input.vNormalWS);
	float3 vTangentWS = normalize(input.vTangentWS);
	float3 vBinormalWS = normalize(input.vBinormalWS);
	float3x3 mTangentToWorld = float3x3(vTangentWS, vBinormalWS, vNormalWS);

	float3 vNormalTS = (NormalMap.Sample(Sampler, input.vTexCoord).xyz * 2.0f) - 1.0f;
	float3 vNormal = normalize(mul(vNormalTS, mTangentToWorld));
#else
	float3 vNormal = normalize(input.vNormalWS);
#endif

	float3 vEmissive = EmissiveColor;

#if SPECULAR_MAPPED
	float3 vSpecularColor = SpecularMap.Sample(Sampler, input.vTexCoord).rgb;
#else
	float3 vSpecularColor = SpecularColor;
#endif

	float fSpecularIntensity = max((vSpecularColor.r + vSpecularColor.g + vSpecularColor.b) / 3.0f, EPSILON);
	float fSpecularPower = max(SpecularPower, EPSILON);

    // RT0 =       Diffuse.r	| Diffuse.g		| Diffuse.b		| Specular Intensity
    // RT1 =       Normal.x		| Normal.y		| Normal.z		| Specular Power
    // RT2 =       Emissive.r	| Emissive.g	| Emissive.b	| Material ID
    // RT3 =       Depth		|				|				|
	output.RT0 = float4(vDiffuse.rgb, fSpecularIntensity);
	output.RT1 = float4(vNormal, fSpecularPower);
	output.RT2 = float4(vEmissive, 0.0f);

	return output;
}


