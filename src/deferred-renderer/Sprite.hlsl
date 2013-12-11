struct VS_In_Sprite
{
    float4 vPosition : POSITION;
    float2 vTexCoord : TEXCOORD0;
    float4 vColor    : COLOR0;
};

struct VS_Out_Sprite
{
    float4 vPosition : SV_POSITION;
    float2 vTexCoord : TEXCOORD0;
    float4 vColor    : COLOR0;
};

Texture2D SpriteTexture    : register(t0);
SamplerState SpriteSampler : register(s0);

VS_Out_Sprite VS_Sprite(VS_In_Sprite input)
{
    VS_Out_Sprite output;

    output.vPosition = input.vPosition;
    output.vTexCoord = input.vTexCoord;
    output.vColor = input.vColor;

    return output;
}

float4 PS_Sprite(VS_Out_Sprite input) : SV_TARGET0
{
    float4 vFontColor = SpriteTexture.SampleLevel(SpriteSampler, input.vTexCoord, 0);

    return vFontColor * input.vColor;
}
