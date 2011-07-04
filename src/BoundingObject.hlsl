cbuffer cbBoundingObjectProperties : register(b0)
{
    float4x4 WorldViewProjection;
}

struct VS_In_BoundingObject
{
	float4 vPositionOS	: POSITION;
	float3 vColor		: COLOR;
};

struct VS_Out_BoundingObject
{
	float4 vPositionCS	: SV_POSITION;
	float3 vColor		: COLOR;
};

VS_Out_BoundingObject VS_BoundingObject(VS_In_BoundingObject input)
{
	VS_Out_BoundingObject output;

	output.vPositionCS = mul(input.vPositionOS, WorldViewProjection);
	output.vColor = input.vColor;

	return output;
}

float4 PS_BoundingObject(VS_Out_BoundingObject input) : SV_TARGET0
{
	return float4(input.vColor, 1.0f);
}