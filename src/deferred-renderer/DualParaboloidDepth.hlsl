#ifndef ALPHA_CUTOUT
#define ALPHA_CUTOUT 0
#endif

cbuffer cbDepthProperties : register(b0)
{
    float4x4 WorldViewProjection;
    float Direction;
    float2 CameraClips;
    float Padding;
}

cbuffer cbAlphaCutoutProperties : register(c1)
{
    float AlphaThreshold    : packoffset(c0);
}

struct VS_In_Depth
{
    float4 vPositionOS : POSITION;
#if ALPHA_CUTOUT
    float2 vTexCoord   : TEXCOORD;
#endif
};

struct VS_Out_Depth
{
    float4 vPositionCS  : SV_POSITION;
#if ALPHA_CUTOUT
    float2 vTexCoord    : TEXCOORD;
#endif
};

VS_Out_Depth VS_Depth(VS_In_Depth input)
{
    VS_Out_Depth output;
    //Render with the Dual-Paraboloid distortion

    // Transform to homogeneous clip space.
    output.vPositionCS = mul(input.vPositionOS, WorldViewProjection);
    output.vPositionCS = output.vPositionCS / output.vPositionCS.w;

    output.vPositionCS.z = output.vPositionCS.z * Direction;

    float L = length(output.vPositionCS.xyz);                           // determine the distance between (0,0,0) and the vertex
    output.vPositionCS = output.vPositionCS / L;                        // divide the vertex position by the distance 

    output.vPositionCS.z = output.vPositionCS.z + 1.0f;                 // add the reflected vector to find the normal vector

    output.vPositionCS.x = output.vPositionCS.x / output.vPositionCS.z; // divide x coord by the new z-value
    output.vPositionCS.y = output.vPositionCS.y / output.vPositionCS.z; // divide y coord by the new z-value

    output.vPositionCS.z = (L - CameraClips.x) /                        // scale the depth to [0, 1]
                           (CameraClips.y - CameraClips.x);
    output.vPositionCS.w = 1;                                           // set w to 1 so there is no w divide

#if ALPHA_CUTOUT
    output.vTexCoord = input.vTexCoord;
#endif

    return output;
}

#if ALPHA_CUTOUT
// ALPHA CUTOUTS
Texture2D DiffuseMap    : register(t0);
SamplerState Sampler    : register(s0);

float4 PS_Depth(VS_Out_Depth input) : SV_TARGET
{
    float alpha = DiffuseMap.SampleLevel(Sampler, input.vTexCoord, 0).a;
    clip(alpha - AlphaThreshold);

    return float4(0.0f, 0.0f, 0.0f, 1.0f);
}
#endif
