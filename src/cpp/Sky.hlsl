cbuffer cbSkyProperties : register(b0)
{
	float3 SkyColor;
	float3 SunColor;
	float3 SunDirection;
	float3 Padding;
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

float4 PS_Sky(PS_In_Quad input) : SV_Target0
{
	float depth = SceneDepth.Sample(PointSampler, input.vTexCoord).x;

	[branch]
	if (depth < 1.0f)
	{
		// Scene
		float3 sceneColor = SceneTexture.Sample(PointSampler, input.vTexCoord).rgb;
		return float4(sceneColor, 1.0f);
	}
	else
	{
		// Sky


		return float4(SkyColor, 1.0f);
	}	
}