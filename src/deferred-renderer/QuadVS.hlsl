struct VS_In_Quad
{
    float4 vPosition : POSITION;
    float2 vTexCoord : TEXCOORD0;
};

struct VS_Out_Quad
{
    float4 vPosition	: SV_POSITION;
    float2 vTexCoord	: TEXCOORD0;
	float2 vPosition2	: TEXCOORD1;
};

VS_Out_Quad VS_Quad(VS_In_Quad input)
{
    VS_Out_Quad output;

    output.vPosition = input.vPosition;
    output.vTexCoord = input.vTexCoord;
	output.vPosition2 = input.vPosition.xy;

    return output;
}