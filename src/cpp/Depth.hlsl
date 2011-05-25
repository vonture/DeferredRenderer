cbuffer cbModelProperties : register(b0)
{
    float4x4 WorldViewProjection;
	float4x4 Padding;
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