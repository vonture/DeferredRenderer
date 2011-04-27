texture SceneTexture;
sampler2D SceneSampler = sampler_state
{
    Texture = <SceneTexture>;
    MinFilter = Point;
    MagFilter = Point;
    
    AddressU = Clamp;
    AddressV = Clamp;
};

texture BrightPassTexture;
sampler2D BrightPassSampler = sampler_state
{
    Texture = <BrightPassTexture>;
    MinFilter = Point;
    MagFilter = Point;
    
    AddressU = Clamp;
    AddressV = Clamp;
};

float AverageLuminance;
float MaximumLuminance;

float Exposure;
float GaussianScalar;
float2 BrightPassSize;
float2 InverseBrightPassSize;

float2 TargetSize;

void VS_HDRCombine(	in float4 inPosition	: POSITION,
					in float2 inTexCoord	: TEXCOORD0,
					out float4 outPosition	: POSITION,
					out float2 outTexCoord	: TEXCOORD0 )
{
	outPosition = inPosition;
	outTexCoord = inTexCoord + (0.5f / TargetSize);
}

float4 PS_HDRCombine( in float2 vTexCoord : TEXCOORD0 ) : COLOR0
{
    // Read the HDR value that was computed as part of the original scene
    float3 vSceneColor = tex2D(SceneSampler, vTexCoord).rgb;
            
    // Compute the blur value using a bilinear filter
    // It is worth noting that if the hardware supports linear filtering of a
    // floating point render target that this step can probably be skipped.
    float xWeight = frac( vTexCoord.x / InverseBrightPassSize.x ) - 0.5f;
    float xDir = xWeight;
    xWeight = abs( xWeight );
    xDir /= xWeight;
    xDir *= InverseBrightPassSize.x;

    float yWeight = frac( vTexCoord.y / InverseBrightPassSize.y ) - 0.5f;
    float yDir = yWeight;
    yWeight = abs( yWeight );
    yDir /= yWeight;
    yDir *= InverseBrightPassSize.y;

    // sample the blur texture for the 4 relevant pixels, weighted accordingly
    float3 vBlur = ((1.0f - xWeight) * (1.0f - yWeight))    * tex2D(BrightPassSampler, vTexCoord).rgb;        
    vBlur +=       (xWeight * (1.0f - yWeight))             * tex2D(BrightPassSampler, vTexCoord + float2(xDir, 0.0f)).rgb;
    vBlur +=       (yWeight * (1.0f - xWeight))             * tex2D(BrightPassSampler, vTexCoord + float2(0.0f, yDir)).rgb;
    vBlur +=       (xWeight * yWeight)                      * tex2D(BrightPassSampler, vTexCoord + float2(xDir, yDir)).rgb;
     
	//float3 vBlur = tex2D(BrightPassSampler, vTexCoord).rgb;

    // Compute the actual colour:
    float3 final = vSceneColor + vBlur;
            
    // Reinhard's tone mapping equation (See Eqn#3 from 
    // "Photographic Tone Reproduction for Digital Images" for more details) is:
    //
    //      (      (   Lp    ))
    // Lp * (1.0f +(---------))
    //      (      ((Lm * Lm)))
    // -------------------------
    //         1.0f + Lp
    //
    // Lp is the luminance at the given point, this is computed using Eqn#2 from the above paper:
    //
    //        exposure
    //   Lp = -------- * HDRPixelIntensity
    //          l.r
    //
    // The exposure ("key" in the above paper) can be used to adjust the overall "balance" of 
    // the image. "l.r" is the average luminance across the scene, computed via the luminance
    // downsampling process. 'HDRPixelIntensity' is the measured brightness of the current pixel
    // being processed.
    
    float Lp = (GaussianScalar / AverageLuminance) * max( final.r, max( final.g, final.b ) );
    
    // A slight difference is that we have a bloom component in the final image - this is *added* to the 
    // final result, therefore potentially increasing the maximum luminance across the whole image. 
    // For a bright area of the display, this factor should be the integral of the bloom distribution 
    // multipled by the maximum value. The integral of the gaussian distribution between [-1,+1] should 
    // be AT MOST 1.0; but the sample code adds a scalar to the front of this, making it a good enough
    // approximation to the *real* integral.
    
    float LmSqr = (AverageLuminance + GaussianScalar * MaximumLuminance) *
				  (AverageLuminance + GaussianScalar * MaximumLuminance);
    
    // Compute Eqn#3:
    float toneScalar = ( Lp * ( 1.0f + ( Lp / ( LmSqr ) ) ) ) / ( 1.0f + Lp );
    
	//return float4(vSceneColor, 1.0f);
	//return float4(vBlur, 1.0f);
	//return float4(AverageLuminance, AverageLuminance,AverageLuminance, 1.0f);
	//return float4(vTexCoord.xxx, 1.0f);
    return float4(final * toneScalar, 1.0f);
}

technique HDRCombine
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_HDRCombine();
        PixelShader = compile ps_2_0 PS_HDRCombine();
    }
}