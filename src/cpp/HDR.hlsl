#define GREY float3(0.212671f, 0.715160f, 0.072169f)
#define RGB_TO_CIEXYZ float3x3(0.4124f, 0.3576f, 0.1805f, \
                               0.2126f, 0.7152f, 0.0722f, \
							   0.0193f, 0.1192f, 0.9505f)
#define CIEXYZ_TO_RGB float3x3( 3.2405f, -1.5371f, -0.4985f, \
                               -0.9693f,  1.8760f,  0.0416f, \
							    0.0556f, -0.2040f,  1.0572f)
#define ROE_RODS 0.2f
#define ROE_CONES 0.4f
#define EPSILON 0.0001f
#define BLUR_RADIUS 4

cbuffer cbHDRProperties : register(b0)
{
	float Tau;
	float KeyValue;
	float TimeDelta;
	uint MipLevels;
	float BloomThreshold;
	float BloomMagnitude;
    float BloomBlurSigma;
	float GaussianNumerator;
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
	float3 CIE_XYZ = mul(RGB_TO_CIEXYZ, color);

	return CIE_XYZ.y / (CIE_XYZ.x + CIE_XYZ.y + CIE_XYZ.z);
}

float4 PS_LuminanceMap(PS_In_Quad input) : SV_TARGET0
{
    float3 vSceneColor = Texture0.Sample(LinearSampler, input.vTexCoord).rgb;
	float fCurLum = CalcLuminance(vSceneColor);

	float fPrevLum = exp(Texture1.Sample(PointSampler, input.vTexCoord).x);

	// Adapt the luminance using Pattanaik's technique
	float fRoe = lerp(ROE_RODS, ROE_CONES, Tau);
    float fAdaptedLum = fPrevLum + (fCurLum - fPrevLum) * (1.0f - exp(-TimeDelta * fRoe));

    return float4(log(fAdaptedLum), 0.0f, 0.0f, 1.0f);
}

float CalculateScaledLuminance(float3 vSceneColor, float fAvgLum)
{
	float fLumGrey = CalcLuminance(GREY);
	float fLumScene = CalcLuminance(vSceneColor);

	return (fLumScene * fLumGrey) / fAvgLum;
}

float CalculateCompressedLuminance(float fScaledLum, float fThreshold, float fOffset)
{
	float fLumWhite = KeyValue;

	float fNumerator = max(fScaledLum * (1.0f + (fScaledLum / (fLumWhite * fLumWhite))) - fThreshold, 0.0f);
	float fDenominator = fOffset + fScaledLum;

	return fNumerator / fDenominator;
}

// Applies exposure and tone mapping to the specific color, and applies
// the threshold to the exposure value.
float3 ToneMap(float3 vSceneColor, float fAvgLum, float fThreshold, float fOffset)
{
	float fScaledLum = CalculateScaledLuminance(vSceneColor, fAvgLum);
	float fCompressedLum = CalculateCompressedLuminance(fScaledLum, fThreshold, fOffset);

	return float3(fCompressedLum * vSceneColor);	
}

// Applies exposure and tone mapping to the input, and combines it with the
// results of the bloom pass
float4 PS_ToneMap(PS_In_Quad input) : SV_TARGET0
{	
    // Tone map the primary input
	float3 vSceneColor = Texture0.Sample(PointSampler, input.vTexCoord).rgb;
    float fAvgLum = exp(Texture1.SampleLevel(PointSampler, input.vTexCoord, MipLevels).x);    
    	
	float3 vBloomColor = Texture2.Sample(LinearSampler, input.vTexCoord).rgb * BloomMagnitude;

	float3 vFinalColor = ToneMap(vSceneColor, fAvgLum, 0.0f, 1.0f) + vBloomColor;

	return float4(vFinalColor, 1.0f);
}

float4 PS_Threshold(PS_In_Quad input) : SV_TARGET0
{
    float3 vSceneColor = Texture0.Sample(LinearSampler, input.vTexCoord).rgb;
	float fAvgLum = exp(Texture1.SampleLevel(PointSampler, input.vTexCoord, MipLevels).x);
		
    float3 vFinalColor = ToneMap(vSceneColor, fAvgLum, BloomThreshold, 1.0f);

    return float4(vFinalColor, 1.0f);
}

float4 PS_Scale(PS_In_Quad input) : SV_TARGET0
{
	return Texture0.Sample(LinearSampler, input.vTexCoord);
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