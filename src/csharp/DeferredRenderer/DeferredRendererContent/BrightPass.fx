static const int NUM_BRIGHTPASS_OFFSETS = 5;
float2 LuminanceOffsets[NUM_BRIGHTPASS_OFFSETS];

float LuminanceThreshold;

static const float3 Grey = float3(0.212671f, 0.715160f, 0.072169f); 

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

void VS_BrightPass(	in float4 inPosition	: POSITION,
					in float2 inTexCoord	: TEXCOORD0,
					out float4 outPosition	: POSITION,
					out float2 outTexCoord	: TEXCOORD0 )
{
	outPosition = inPosition;
	outTexCoord = inTexCoord + (0.5f / TargetSize);
}

float4 PS_BrightPass(in float2 in_vTexCoord : TEXCOORD0) : COLOR
{
	float4 average = 0.0f;

    for( int i = 0; i < NUM_BRIGHTPASS_OFFSETS; i++ )
    {
        average += tex2D(SceneSampler, in_vTexCoord + LuminanceOffsets[i]);
    }
	average /= (float)NUM_BRIGHTPASS_OFFSETS;

	float luminance = dot(average.rgb, Grey);
	if (luminance < LuminanceThreshold)
	{
		average = 0.0f;
	}

	//return float4(in_vTexCoord.xy, 0.0f, 1.0f);
	return average;
}

technique BrightPass
{
    pass Pass1
    {
		VertexShader = compile vs_2_0 VS_BrightPass();
        PixelShader = compile ps_2_0 PS_BrightPass();
    }
}