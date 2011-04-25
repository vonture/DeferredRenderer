struct VS_In_Clear
{
	float3 vPosition : POSITION0;
};

struct VS_Out_Clear
{
	float4 vPositionCS : POSITION0;
};

struct PS_Out_Clear
{    
	float4 RT0 : COLOR0;
	float4 RT1 : COLOR1;
	float4 RT2 : COLOR2;
	float4 RT3 : COLOR3;
};

VS_Out_Clear VS_Clear(VS_In_Clear input)
{
	VS_Out_Clear output;

	output.vPositionCS = float4(input.vPosition, 1.0f);

	return output;
}

PS_Out_Clear PS_Clear()
{
    PS_Out_Clear output;

    output.RT0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
	output.RT1 = float4(0.5f, 0.5f, 0.5f, 0.0f);
	output.RT2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
	output.RT3 = float4(0.0f, 0.0f, 0.0f, 0.0f);

    return output;
}

technique GBufferClear
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Clear();
        PixelShader = compile ps_2_0 PS_Clear();
    }
}