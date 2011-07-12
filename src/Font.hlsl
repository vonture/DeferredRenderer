struct VS_In_Font
{
    float4 vPosition : POSITION;
    float2 vTexCoord : TEXCOORD0;
	float4 vColor	 : COLOR0;
};

struct VS_Out_Font
{
	float4 vPosition : SV_POSITION;
	float2 vTexCoord : TEXCOORD0;
	float4 vColor	 : COLOR0;
};

Texture2D FontTexture : register(t0);
SamplerState FontSampler : register(s0);

VS_Out_Font VS_Font(VS_In_Font input)
{
    VS_Out_Font output;

    output.vPosition = input.vPosition;
    output.vTexCoord = input.vTexCoord;
	output.vColor = input.vColor;

    return output;
}

float4 PS_Font(VS_Out_Font input) : SV_TARGET0
{
	float4 vFontColor = FontTexture.SampleLevel(FontSampler, input.vTexCoord, 0);
	
	return vFontColor * input.vColor;
}