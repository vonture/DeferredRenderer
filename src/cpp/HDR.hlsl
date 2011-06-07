static const float3 Grey = float3(0.212671f, 0.715160f, 0.072169f); 
static const float Epsilon = 0.0001f;

cbuffer cbHDRProperties : register(b0)
{
	float AdaptationRate;
	float KeyValue;
	float TimeDelta;
	uint MipLevels;
}

Texture2D Texture0 : register(t0);
Texture2D Texture1 : register(t1);

SamplerState PointSampler : register(s0);
SamplerState LinearSampler : register(s1);

struct PS_In_Quad
{
    float4 vPosition	: SV_POSITION;
    float2 vTexCoord	: TEXCOORD0;
	float2 vPosition2	: TEXCOORD1;
};

float CalcLuminance(float3 color)
{
    return max(dot(color, Grey), Epsilon);
}

float4 PS_LuminanceMap(PS_In_Quad input) : SV_TARGET0
{
    float3 sceneColor = Texture0.Sample(LinearSampler, input.vTexCoord).rgb;
	float curLum = CalcLuminance(sceneColor);

	float prevLum = exp(Texture1.Sample(PointSampler, input.vTexCoord).x);

	// Adapt the luminance using Pattanaik's technique
    float adaptedLum = prevLum + (curLum - prevLum) * (1.0f - exp(-TimeDelta * AdaptationRate));

    return float4(log(adaptedLum), 1.0f, 1.0f, 1.0f);
}

// Applies the filmic curve from John Hable's presentation
float3 ToneMapFilmicALU(float3 color)
{
    color = max(0.0f, color - 0.004f);
    color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f) + 0.06f);

    // result has 1/2.2 baked in
    return pow(color, 2.2f);
}

// Determines the color based on exposure settings
float3 CalcExposedColor(float3 color, float avgLuminance, float threshold)
{
	// Use geometric mean
	avgLuminance = max(avgLuminance, Epsilon);
	float linearExposure = (KeyValue / avgLuminance);
	float exposure = log2(max(linearExposure, Epsilon));
    exposure -= threshold;
    return exp2(exposure) * color;
}

// Applies exposure and tone mapping to the input, and combines it with the
// results of the bloom pass
float4 PS_ToneMap(PS_In_Quad input) : SV_Target0
{	
    // Tone map the primary input
    float avgLuminance = exp(Texture1.SampleLevel(PointSampler, input.vTexCoord, MipLevels).x);
    float3 sceneColor = Texture0.Sample(LinearSampler, input.vTexCoord).rgb;
    	
	float pixelLuminance = CalcLuminance(sceneColor);

	float3 final = CalcExposedColor(sceneColor, avgLuminance, 0);
	final = ToneMapFilmicALU(final);
	
	/*
    // Sample the bloom
    float3 bloom = InputTexture2.Sample(LinearSampler, input.TexCoord).rgb;
    bloom = bloom * BloomMagnitude;

    // Add in the bloom
	color = color + bloom;
	*/

	return float4(final, 1.0f);
}