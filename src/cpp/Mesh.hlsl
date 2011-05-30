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
	float2 vTexCoord	: TEXCOORD0;
};

struct VS_Out_Mesh
{
	float4 vPositionCS	: SV_POSITION;
	float2 vTexCoord	: TEXCOORD0;
	float3 vNormalWS	: TEXCOORD1;
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
	output.vTexCoord = input.vTexCoord;

    return output;
}

PS_Out_Mesh PS_Mesh(VS_Out_Mesh input)
{
	PS_Out_Mesh output;

	float4 vDiffuse = DiffuseMap.Sample(Sampler, input.vTexCoord);	
	//float3 vNormal = NormalMap.Sample(Sampler, input.vTexCoord);
	float3 vNormal = normalize(input.vNormalWS);

    // RT0 =       Diffuse.r           | Diffuse.g         | Diffuse.b     | Specular Intensity
    // RT1 =       Normal.x            | Normal.y          | Normal.z      | Specular Power
    // RT2 =                           | Ambient Occlusion | Translucency  | Material ID
	output.RT0 = float4(vDiffuse.rgb, 128.0f);
	output.RT1 = float4(vNormal, 40.0f);
	output.RT2 = float4(0.0f, 0.02f, 0.0f, 0.0f);

	return output;
}


