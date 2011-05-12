cbuffer cbModelProperties : register(b0)
{
    float4x4 World					: packoffset(c0);
    float4x4 WorldViewProjection	: packoffset(c4);
}

Texture2D DiffuseMap	: register(t0);
Texture2D NormalMap		: register(t1);

SamplerState	LinearSampler	: register(s0);

struct VS_In_Model
{
	float4 vPositionOS	: POSITION;
	float3 vNormalOS	: NORMAL;
	float2 vTexCoord	: TEXCOORD0;
};

struct VS_Out_Model
{
	float4 vPositionCS	: SV_POSITION;
	float2 vTexCoord	: TEXCOORD0;
	float3 vNormalWS	: TEXCOORD1;
};

struct PS_Out_Model
{    
	float4 RT0 : SV_TARGET0;
	float4 RT1 : SV_TARGET1;
	float4 RT2 : SV_TARGET2;
};

struct VS_In_ModelDepth
{
	float4 vPositionOS	: POSITION;
};

struct VS_Out_ModelDepth
{
	float4 vPositionCS	: SV_POSITION;
};

VS_Out_Model VS_Model(VS_In_Model input)
{
    VS_Out_Model output;

    output.vPositionCS = mul(input.vPositionOS, WorldViewProjection);
	output.vNormalWS = mul(input.vNormalOS, (float3x3)World);
	output.vTexCoord = input.vTexCoord;

    return output;
}

PS_Out_Model PS_Model(VS_Out_Model input)
{
	PS_Out_Model output;

	float4 vDiffuse = DiffuseMap.Sample(LinearSampler, input.vTexCoord);	
	//float3 vNormal = NormalMap.Sample(LinearSampler, input.vTexCoord);
	float3 vNormal = normalize(input.vNormalWS);

	output.RT0 = float4(vDiffuse.rgb, 1.0f);
	output.RT1 = float4(vNormal, 1.0f);
	output.RT2 = float4(0.0f, 1.0f, 0.0f, 0.0f);

	return output;
}


VS_Out_ModelDepth VS_ModelDepth(VS_In_ModelDepth input)
{
    VS_Out_ModelDepth output;

    output.vPositionCS = mul(input.vPositionOS, WorldViewProjection);

    return output;
}


float4 PS_ModelDepth(VS_Out_ModelDepth input) : SV_TARGET0
{
	float fDepth = input.vPositionCS.z / input.vPositionCS.w;

	return float4(fDepth, fDepth * fDepth, 0.0f, 0.0f);
}