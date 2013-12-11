cbuffer cbBoundingObjectProperties : register(b0)
{
    float4x4 WorldViewProjection;
}

cbuffer cbBoundingObjectColor : register(b0)
{
    float4 Color;
}

struct VS_In_BoundingObject
{
    float4 vPositionOS : POSITION;
};

struct VS_Out_BoundingObject
{
    float4 vPositionCS : SV_POSITION;
};

VS_Out_BoundingObject VS_BoundingObject(VS_In_BoundingObject input)
{
    VS_Out_BoundingObject output;

    output.vPositionCS = mul(input.vPositionOS, WorldViewProjection);

    return output;
}

float4 PS_BoundingObject(VS_Out_BoundingObject input) : SV_TARGET0
{
    return Color;
}
