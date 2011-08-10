#define GREY float3(0.212671f, 0.715160f, 0.072169f)
#define ROE_RODS 0.2f
#define ROE_CONES 0.4f
#define EPSILON 0.0001f
#define BLUR_RADIUS 4

cbuffer cbHDRProperties : register(b0)
{
	float Tau;
	float LuminanceWhitePerc;
	float ExposureKey;
	float TimeDelta;
	uint MipLevels;
	float BloomThreshold;
	float BloomMagnitude;
    float BloomBlurSigma;
	float GaussianNumerator;
	float3 Padding;
}

Texture2D Texture0 : register(t0);
Texture2D Texture1 : register(t1);
Texture2D Texture2 : register(t2);
Texture3D Texture3 : register(t3);

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
	return max(dot(color, GREY), EPSILON);
}

float4 PS_LuminanceMap(PS_In_Quad input) : SV_TARGET0
{
    float3 vSceneColor = Texture0.SampleLevel(LinearSampler, input.vTexCoord, 0).rgb;
	float fCurLum = CalcLuminance(vSceneColor);

	float fPrevLum = exp(Texture1.SampleLevel(PointSampler, input.vTexCoord, MipLevels).x);

	// Adapt the luminance using Pattanaik's technique
	float fRoe = lerp(ROE_RODS, ROE_CONES, Tau);
    float fAdaptedLum = fPrevLum + (fCurLum - fPrevLum) * (1.0f - exp(-TimeDelta * fRoe));

    return float4(log(fAdaptedLum), 0.0f, 0.0f, 1.0f);
}

float CalculateCompressedLuminance(float fAvgLum, float3 vSceneColor, float fThreshold, float fOffset)
{
	float fLumPixel = CalcLuminance(vSceneColor);
	float fLumWhite = LuminanceWhitePerc * fAvgLum;

	float L = (ExposureKey / fAvgLum) * fLumPixel;
	float Ld = max(L * (1.0f + (L / (fLumWhite * fLumWhite))) - fThreshold, 0.0f) / (fOffset + L);

	return max(Ld, EPSILON);
}

// Applies exposure and tone mapping to the specific color, and applies
// the threshold to the exposure value.
float3 ToneMap(float3 vSceneColor, float fAvgLum, float fThreshold, float fOffset)
{
	float fCompressedLum = CalculateCompressedLuminance(fAvgLum, vSceneColor, fThreshold, fOffset);

	return float3(fCompressedLum * vSceneColor);	
}

float3 ColorGrade(float3 vSceneColor)
{
	return Texture3.SampleLevel(LinearSampler, vSceneColor, 0).rgb;
}

// Applies exposure and tone mapping to the input, and combines it with the
// results of the bloom pass
float4 PS_ToneMap(PS_In_Quad input) : SV_TARGET0
{	
    // Tone map the primary input
	float3 vSceneColor = Texture0.SampleLevel(PointSampler, input.vTexCoord, 0).rgb;
    float fAvgLum = exp(Texture1.SampleLevel(PointSampler, input.vTexCoord, MipLevels).x);    
    	
	float3 vBloomColor = Texture2.SampleLevel(LinearSampler, input.vTexCoord, 0).rgb * BloomMagnitude;
	float3 vToneMappedColor = ToneMap(vSceneColor, fAvgLum, 0.0f, 1.0f) + vBloomColor;	
	float3 vGradedColor = ColorGrade(vToneMappedColor);

	return float4(vGradedColor, 1.0f);
}

float4 PS_Threshold(PS_In_Quad input) : SV_TARGET0
{
    float3 vSceneColor = Texture0.SampleLevel(LinearSampler, input.vTexCoord, 0).rgb;
	float fAvgLum = exp(Texture1.SampleLevel(PointSampler, input.vTexCoord, MipLevels).x);
		
    float3 vFinalColor = ToneMap(vSceneColor, fAvgLum, BloomThreshold, 1.0f);
	
    return float4(vFinalColor, 1.0f);
}

float4 PS_Scale(PS_In_Quad input) : SV_TARGET0
{
	return Texture0.SampleLevel(LinearSampler, input.vTexCoord, 0);
}

// Calculates the gaussian blur weight for a given distance and sigmas
float CalcGaussianWeight(int sampleDist, float sigma)
{
	return (GaussianNumerator * exp(-(sampleDist * sampleDist) / (2 * sigma * sigma)));
}

// Performs a gaussian blur in one direction
float4 Blur(float2 texCoord, int2 direction, float sigma)
{
    float4 color = 0;
    for (int i = -BLUR_RADIUS; i < BLUR_RADIUS; i++)
    {
		float weight = CalcGaussianWeight(i, sigma);
		float4 sample = Texture0.SampleLevel(PointSampler, texCoord, 0, direction * i);
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