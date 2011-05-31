cbuffer cbModelProperties : register(b0)
{
    float4x4 World					: packoffset(c0);
    float4x4 WorldViewProjection	: packoffset(c4);
}

Texture2D DiffuseMap	: register(t0);
Texture2D NormalMap		: register(t1);

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

	float3 vNormalWS = normalize(input.vNormalWS);
	float3 vTangentWS = normalize(input.vTangentWS);
	float3 vBinormalWS = normalize(input.vBinormalWS);
	float3x3 mTangentToWorld = float3x3(vTangentWS, vBinormalWS, vNormalWS);

	float3 vNormalTS = (NormalMap.Sample(Sampler, input.vTexCoord) * 2.0f) - 1.0f;
	float3 vNormal = normalize(mul(vNormalTS, mTangentToWorld));

	float4 vDiffuse = DiffuseMap.Sample(Sampler, input.vTexCoord);		

    // RT0 =       Diffuse.r           | Diffuse.g         | Diffuse.b     | Specular Intensity
    // RT1 =       Normal.x            | Normal.y          | Normal.z      | Specular Power
    // RT2 =                           | Ambient Occlusion | Translucency  | Material ID
	output.RT0 = float4(vDiffuse.rgb, 4.0f);
	output.RT1 = float4(vNormal, 1.0f);
	output.RT2 = float4(0.0f, 0.02f, 0.0f, 0.0f);

	return output;
}


