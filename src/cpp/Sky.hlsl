cbuffer cbSkyProperties : register(cb0)
{
	uint SunEnabled					: packoffset(c0.x);
	float SunWidth					: packoffset(c0.y);
	float3 SkyColor					: packoffset(c1.x);
	float3 SunColor					: packoffset(c2.x);
	float3 SunDirection				: packoffset(c3.x);	
	float3 CameraPosition			: packoffset(c4.x);
	float4x4 InverseViewProjection	: packoffset(c5.x);
}

struct PS_In_Quad
{
    float4 vPosition	: SV_POSITION;
    float2 vTexCoord	: TEXCOORD0;
	float2 vPosition2	: TEXCOORD1;
};

Texture2D SceneTexture : register(t0);
Texture2D SceneDepth : register(t1);

SamplerState PointSampler : register(s0);

float4 GetPositionWS(float2 vPositionCS, float fDepth)
{
	float4 vPositionWS = mul(float4(vPositionCS.x, vPositionCS.y, fDepth, 1.0f), InverseViewProjection);
	vPositionWS.xyz = vPositionWS.xyz / vPositionWS.www;
	vPositionWS.w = 1.0f;

	return vPositionWS;
}

float AngleBetween(float3 dirA, float3 dirB)
{
	return acos(dot(dirA, dirB));
}

//-------------------------------------------------------------------------------------------------
// Uses the CIE Clear Sky model to compute a color for a pixel, given a direction + sun direction
//-------------------------------------------------------------------------------------------------
float3 CIEClearSky(float3 dir, float3 sunDir)
{
	float3 skyDir = float3(dir.x, abs(dir.y), dir.z);
	float gamma = AngleBetween(skyDir, sunDir);
	float S = AngleBetween(sunDir, float3(0, 1, 0));
	float theta = AngleBetween(skyDir, float3(0, 1, 0));

	float cosTheta = cos(theta);
	float cosS = cos(S);
	float cosGamma = cos(gamma);

	const float b = -0.32f;
	const float c = 10.0f;
	const float d = -3.0f;
	const float e = 0.45f;

	float num = (1.0f + c * exp(d * gamma) + e * cosGamma * cosGamma) * (1 - exp(b / cosTheta));
	float denom = (1.0f + c * exp(d * S) + e * cosS * cosS) * (1 - exp(b));

	float lum = num / denom;

	float3 finalSkyColor = SkyColor;

	// Draw a circle for the sun
	[flatten]
	if (SunEnabled)
	{
		float sunGamma = AngleBetween(dir, sunDir);
		finalSkyColor = lerp(SunColor * 150.0f, SkyColor, saturate(abs(sunGamma) / 0.05f));
	}

	return max(finalSkyColor * lum, 0.0f);
}

float4 PS_Sky(PS_In_Quad input) : SV_TARGET0
{
	float fDepth = SceneDepth.Sample(PointSampler, input.vTexCoord).x;

	[branch]
	if (fDepth < 1.0f) // Scene
	{		
		float3 sceneColor = SceneTexture.Sample(PointSampler, input.vTexCoord).rgb;
		return float4(sceneColor, 1.0f);
	}
	else // Sky
	{
		float4 vPositionWS = GetPositionWS(input.vPosition2, 1.0f);
		float3 vSkyDirection = normalize(vPositionWS.xyz - CameraPosition.xyz);

		float3 CIESkyColor = CIEClearSky(vSkyDirection, SunDirection.xyz);

		return float4(CIESkyColor, 1.0f);
	}	
}