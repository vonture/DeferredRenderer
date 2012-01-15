cbuffer cbAlphaCutoutProperties : register(c1)
{
    float AlphaThreshold	: packoffset(c0);
}

// NO ALPHA CUTOUTS
struct VS_In_DepthNoAlpha
{
	float4 vPositionOS	: POSITION;
	float4x4 mWVP		: WVP;
};

struct VS_Out_DepthNoAlpha
{
	float4 vPositionCS	: SV_POSITION;
};

VS_Out_DepthNoAlpha VS_DepthNoAlpha(VS_In_DepthNoAlpha input)
{
    VS_Out_DepthNoAlpha output;

    output.vPositionCS = mul(input.vPositionOS, input.mWVP);

    return output;
}

// ALPHA CUTOUTS
Texture2D DiffuseMap	: register(t0);
SamplerState Sampler	: register(s0);

struct VS_In_DepthAlpha
{
	float4 vPositionOS	: POSITION;
	float2 vTexCoord	: TEXCOORD;
	float4x4 mWVP		: WVP;
};

struct VS_Out_DepthAlpha
{
	float4 vPositionCS	: SV_POSITION;
	float2 vTexCoord	: TEXCOORD;
};

VS_Out_DepthAlpha VS_DepthAlpha(VS_In_DepthAlpha input)
{
    VS_Out_DepthAlpha output;

    output.vPositionCS = mul(input.vPositionOS, input.mWVP);
	output.vTexCoord = input.vTexCoord;

    return output;
}

float4 PS_DepthAlpha(VS_Out_DepthAlpha input) : SV_TARGET0
{
	float alpha = DiffuseMap.SampleLevel(Sampler, input.vTexCoord, 0).a;
	clip(alpha - AlphaThreshold);

	return float4(0.0f, 0.0f, 0.0f, 1.0f);
}