struct PS_In_Quad
{
    float4 vPosition    : SV_POSITION;
    float2 vTexCoord    : TEXCOORD0;
    float2 vPosition2    : TEXCOORD1;
};

Texture2D Texture0    : register(t0);
SamplerState Sampler0 : register(s0);

float4 PS_Copy(PS_In_Quad input) : SV_TARGET0
{
    return Texture0.SampleLevel(Sampler0, input.vTexCoord, 0);
}
