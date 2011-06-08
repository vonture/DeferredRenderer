static const float3 Grey = float3(0.212671f, 0.715160f, 0.072169f); 
static const float Epsilon = 0.0001f;
static const float Pi = 3.14159f;

cbuffer cbHDRProperties : register(b0)
{
	float AdaptationRate;
	float KeyValue;
	float TimeDelta;
	uint MipLevels;
	float BloomThreshold;
	float BloomMagnitude;
    float BloomBlurSigma;
	float Padding;
}

Texture2D Texture0 : register(t0);
Texture2D Texture1 : register(t1);
Texture2D Texture2 : register(t2);

SamplerState PointSampler  : register(s0);
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
	float exposure = log2(max(linearExposure, Epsilon)) - threshold;
    return exp2(exposure) * color;
}

// Applies exposure and tone mapping to the specific color, and applies
// the threshold to the exposure value.
float3 ToneMap(float3 color, float avgLuminance, float threshold)
{
    float pixelLuminance = CalcLuminance(color);
    color = CalcExposedColor(color, avgLuminance, threshold);
	color = ToneMapFilmicALU(color);
    return color;
}

// Applies exposure and tone mapping to the input, and combines it with the
// results of the bloom pass
float4 PS_ToneMap(PS_In_Quad input) : SV_TARGET0
{	
    // Tone map the primary input
	float3 sceneColor = Texture0.Sample(PointSampler, input.vTexCoord).rgb;
    float avgLuminance = exp(Texture1.SampleLevel(LinearSampler, input.vTexCoord, MipLevels).x);    
    	
	float3 bloom = Texture2.Sample(LinearSampler, input.vTexCoord).rgb * BloomMagnitude;

	float3 finalColor = ToneMap(sceneColor, avgLuminance, 0) + bloom;

	return float4(finalColor, 1.0f);
}

float4 PS_Threshold(PS_In_Quad input) : SV_TARGET0
{
    float3 sceneColor = Texture0.Sample(LinearSampler, input.vTexCoord).rgb;

    // Tone map it to threshold
    float avgLuminance = exp(Texture1.SampleLevel(LinearSampler, input.vTexCoord, MipLevels).x);
    float3 finalColor = ToneMap(sceneColor, avgLuminance, BloomThreshold);

    return float4(finalColor, 1.0f);
}

float4 PS_Scale(PS_In_Quad input) : SV_TARGET0
{
	return Texture0.Sample(LinearSampler, input.vTexCoord);
}

// Calculates the gaussian blur weight for a given distance and sigmas
float CalcGaussianWeight(int sampleDist, float sigma)
{
	float g = 1.0f / sqrt(2.0f * Pi * sigma * sigma);
	return (g * exp(-(sampleDist * sampleDist) / (2 * sigma * sigma)));
}

// Performs a gaussian blur in one direction
float4 Blur(float2 texCoord, int2 direction, float sigma)
{
	const int Radius = 6;

    float4 color = 0;
    for (int i = -Radius; i < Radius; i++)
    {
		float weight = CalcGaussianWeight(i, sigma);
		float4 sample = Texture0.Sample(PointSampler, texCoord, direction * i);
		color += sample * weight;
    }

    return color;
}

// Horizontal gaussian blur
float4 PS_BlurHorizontal(PS_In_Quad input) : SV_TARGET0
{
    return Blur(input.vTexCoord, int2(1, 0), BloomBlurSigma);
}

// Vertical gaussian blur
float4 PS_BlurVertical(PS_In_Quad input) : SV_TARGET0
{
    return Blur(input.vTexCoord, int2(0, 1), BloomBlurSigma);
}