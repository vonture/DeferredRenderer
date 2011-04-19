float4x4 World;
float4x4 View;
float4x4 Projection;

struct VS_In_Depth
{
	float4 vPositionOS	: POSITION;
};

struct VS_Out_Depth
{
	float4 vPositionCS	: POSITION;
	float2 vDepthCS		: TEXCOORD0;
};

struct PS_In_Model
{
	float2 vDepthCS		: TEXCOORD0;
};

VS_Out_Depth VS_Depth(VS_In_Depth input)
{
    VS_Out_Depth output;

    float4x4 wvp = mul(World, mul(View, Projection));
	output.vPositionCS = mul(input.vPositionOS, wvp);
	output.vDepthCS = output.vPositionCS.zw;

    return output;
}

float4 PS_Depth(PS_In_Model input) : COLOR0
{
    float fDepth = input.vDepthCS.x / input.vDepthCS.y;
	return float4(fDepth, fDepth * fDepth, 1.0f, 1.0f);
}

technique DepthMap
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Depth();
        PixelShader = compile ps_2_0 PS_Depth();
    }
}
