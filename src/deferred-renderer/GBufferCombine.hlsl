Texture2D Texture0 : register(t0); // Diffuse
Texture2D Texture1 : register(t1); // Emissive
Texture2D Texture2 : register(t2); // Light Buffer

SamplerState PointSampler : register(s0);

struct PS_In_Combine
{
    float4 vPosition	: SV_POSITION;
    float2 vTexCoord	: TEXCOORD0;
	float2 vPosition2	: TEXCOORD1;
};

float4 PS_Combine(PS_In_Combine input) : SV_TARGET0
{
	float3 vDiffuse = Texture0.Sample(PointSampler, input.vTexCoord).rgb;
	float3 vEmissive = Texture1.Sample(PointSampler, input.vTexCoord).rgb;

	float4 vLightData = Texture2.Sample(PointSampler, input.vTexCoord);
	float3 vLightColor = vLightData.rgb;
	float fSpecular = vLightData.a;

	float3 vFinalColour = (vEmissive) +
						  (vLightColor * vDiffuse) + 
						  (fSpecular * vLightColor * vDiffuse);

	return float4(vFinalColour, 1.0f);
}