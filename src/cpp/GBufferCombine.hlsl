Texture2D RT0 : register(t0);
Texture2D RT1 : register(t1);
Texture2D RT2 : register(t2);
Texture2D RT3 : register(t3);

Texture2D LightMap : register(t4);

SamplerState LinearSampler : register(s0);

struct PS_In_Combine
{
    float4 vPosition : SV_POSITION;              
    float2 vTexCoord : TEXCOORD0;
};

float4 PS_Combine(PS_In_Combine input) : SV_TARGET0
{
	float4 rt0Sample = RT0.Sample(LinearSampler, input.vTexCoord);
	float4 rt1Sample = RT1.Sample(LinearSampler, input.vTexCoord);
	float4 rt2Sample = RT2.Sample(LinearSampler, input.vTexCoord);
	float4 rt3Sample = RT3.Sample(LinearSampler, input.vTexCoord);

	return float4(0.5f * (rt1Sample.xyz + 1.0f), 1.0f);
}