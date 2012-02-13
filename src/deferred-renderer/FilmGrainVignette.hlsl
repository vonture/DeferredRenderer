#ifndef FILM_GRAIN_ENABLED
#define FILM_GRAIN_ENABLED 1
#endif

#ifndef VIGNETTE_ENABLED
#define VIGNETTE_ENABLED 1
#endif

cbuffer cbProperties : register(b0)
{
	float DeltaTime;
	float FilmGrainStrength;
	float VignetteStart;
	float VignetteStrength;
}

Texture2D SceneTexture : register(t0);

SamplerState PointSampler : register(s0);

struct PS_In_FilmGrainVignette
{
    float4 vPosition	: SV_POSITION;
    float2 vTexCoord	: TEXCOORD0;
	float2 vPosition2	: TEXCOORD1;
};

float4 PS_FilmGrainVignette(PS_In_FilmGrainVignette input) : SV_TARGET0
{
	float4 vSceneColor = SceneTexture.Sample(PointSampler, input.vTexCoord);

#if FILM_GRAIN_ENABLED
	float x = (input.vTexCoord.x + 4.0f) * (input.vTexCoord.y + 4.0f) * (DeltaTime + 10.0f);
    float fGrain = (fmod((fmod(x, 13.0f) + 1.0f) * (fmod(x, 123.0f) + 1.0f), 0.01) - 0.005f) * FilmGrainStrength;

	vSceneColor = saturate(vSceneColor + fGrain);
#endif

#if VIGNETTE_ENABLED
	float fCenterDist = length(2.0f * (0.5f - input.vTexCoord));
	float fVignette = saturate(1.0f + VignetteStart - (fCenterDist * VignetteStrength));

	vSceneColor = saturate(vSceneColor * fVignette);
#endif

	return vSceneColor;
}