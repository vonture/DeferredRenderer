static const int NUM_DOWNSAMPLE_OFFSETS = 9;
float2 DownSampleOffsets[NUM_DOWNSAMPLE_OFFSETS];

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

void VS_DownSample(in float4 inPosition		: POSITION,
				   in float2 inTexCoord		: TEXCOORD0,
				   out float4 outPosition	: POSITION,
				   out float2 outTexCoord	: TEXCOORD0 )
{
    outPosition = inPosition;
	outTexCoord = inTexCoord + (0.5f / TargetSize);
}

float4 PS_GreyScaleDownSample(in float2 vTexCoord : TEXCOORD0 ) : COLOR0
{
    // Compute the average of the 4 necessary samples
    float average = 0.0f;
    float maximum = -1e20;
    float4 color = 0.0f;

    for( int i = 0; i < NUM_DOWNSAMPLE_OFFSETS; i++ )
    {
        color = tex2D(SceneSampler, vTexCoord + DownSampleOffsets[i]);

        float greyOffset = dot(color.rgb, Grey);
		//float greyOffset = max(color.r, max(color.g, color.b));
		                 
        maximum = max(maximum, greyOffset);
        average += greyOffset;
    }
        
    average /= (float)NUM_DOWNSAMPLE_OFFSETS;
        
	// Output the luminance to the render target
    return float4(average, maximum, 0.0f, 1.0f);
}

float4 PS_DownSample( in float2 vTexCoord : TEXCOORD0 ) : COLOR0
{    
    // Compute the average of the 10 necessary samples
    float2 value = 0.0f;
    float maximum = -1e20;
    float average = 0.0f;

    for( int i = 0; i < NUM_DOWNSAMPLE_OFFSETS; i++ )
    {
        value = tex2D(SceneSampler, vTexCoord + DownSampleOffsets[i]).rg;
        average += value.r;
        maximum = max(maximum, value.g);
    }
        
    // We've just taken 9 samples from the high resolution texture, so compute the
    // actual average that is written to the lower resolution texture (render target).
    average /= (float)NUM_DOWNSAMPLE_OFFSETS;
        
    // Return the new average luminance
    return float4(average, maximum, 0.0f, 1.0f);
}

technique GreyScaleDownSample
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_DownSample();
        PixelShader = compile ps_2_0 PS_GreyScaleDownSample();
    }
}

technique DownSample
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_DownSample();
        PixelShader = compile ps_2_0 PS_DownSample();
    }
}

