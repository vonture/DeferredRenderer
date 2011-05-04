struct VS_In_Quad
{
    float4 vPosition : POSITION;
    float2 vTexCoord : TEXCOORD0;
};

struct VS_Out_Quad
{
    float4 vPosition : SV_POSITION;              
    float2 vTexCoord : TEXCOORD0;
};

VS_Out_Quad VS_Quad(VS_In_Quad Input)
{
    VS_Out_Quad output;

    output.vPosition = Input.vPosition;
    output.vTexCoord = Input.vTexCoord;

    return output;
}