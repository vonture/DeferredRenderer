static const int NUM_DOWNSAMPLE_OFFSETS = 5;
float2 DownSampleOffsets[NUM_DOWNSAMPLE_OFFSETS];

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

void VS_DownSample(in float4 inPosition		: POSITION,
				   in float2 inTexCoord		: TEXCOORD0,
				   out float4 outPosition	: POSITION,
				   out float2 outTexCoord	: TEXCOORD0 )
{
    outPosition = inPosition;
	outTexCoord = inTexCoord + (0.5f / TargetSize);
}

float4 PS_DownSample( in float2 vTexCoord : TEXCOORD0 ) : COLOR
{    
    float4 average = 0.0f;

    for( int i = 0; i < NUM_DOWNSAMPLE_OFFSETS; i++ )
    {
        average += tex2D(SceneSampler, vTexCoord + DownSampleOffsets[i]);
    }
        
    average /= NUM_DOWNSAMPLE_OFFSETS;

    return average;    
}

technique DownSample
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_DownSample();
        PixelShader = compile ps_2_0 PS_DownSample();
    }
}