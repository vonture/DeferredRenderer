#define SAMPLE_COUNT 9

float2 Offsets[SAMPLE_COUNT];
float Weights[SAMPLE_COUNT];

float2 SceneSize;
float2 TargetSize;

texture SceneTexture;
sampler2D SceneSampler = sampler_state
{
    Texture = <SceneTexture>;
    MinFilter = Point;
    MagFilter = Point;
    
    AddressU = Clamp;
    AddressV = Clamp;
};

void VS_Blur(in float4 inPosition :		POSITION,
			 in float2 inTexCoord :		TEXCOORD0,
			 out float4 outPosition :	POSITION,
			 out float2 outTexCoord :	TEXCOORD0 )
{
    outPosition = inPosition;
	outTexCoord = inTexCoord + (0.5f / TargetSize);
}

float4 PS_Blur(in float4 inPosition :	POSITION,
			    in float2 inTexCoord :	TEXCOORD0) : COLOR0
{
	float4 c = 0;
    
    // Combine a number of weighted image filter taps.
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        c += tex2D(SceneSampler, inTexCoord + Offsets[i]) * Weights[i];
    }
    
    return c;
}

technique Blur
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Blur();
        PixelShader = compile ps_2_0 PS_Blur();
    }
}
