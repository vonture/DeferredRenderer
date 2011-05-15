cbuffer cbModelProperties : register(b0)
{
    float4x4 World					: packoffset(c0);
    float4x4 WorldViewProjection	: packoffset(c4);
}

struct VS_In_Depth
{
	float4 vPositionOS	: POSITION;
};

struct VS_Out_Depth
{
	float4 vPositionCS	: SV_POSITION;
};

VS_Out_Depth VS_Depth(VS_In_Depth input)
{
    VS_Out_Depth output;

    output.vPositionCS = mul(input.vPositionOS, WorldViewProjection);

    return output;
}

float4 PS_Depth(VS_Out_Depth input) : SV_TARGET0
{
	float fDepth = input.vPositionCS.z / input.vPositionCS.w;

	return float4(fDepth, fDepth * fDepth, 0.0f, 0.0f);
}