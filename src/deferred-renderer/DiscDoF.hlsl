#ifndef DOF_SAMPLE_COUNT
#define DOF_SAMPLE_COUNT 16
#endif

cbuffer cbDoFProperties : register(cb0)
{
    float CameraNearClip            : packoffset(c0.x);
    float CameraFarClip             : packoffset(c0.y);
    float FocalDistance             : packoffset(c0.z);
    float FocalFalloffNear          : packoffset(c0.w);
    float FocalFalloffFar           : packoffset(c1.x);
    float CircleOfConfusionScale    : packoffset(c1.y);
    float2 InvSceneResolution       : packoffset(c1.z);
}

cbuffer cbSampleDirections : register(cb1)
{
    float4 SampleDirections[DOF_SAMPLE_COUNT] : packoffset(c0.x);
}

Texture2D DepthTexture : register(t0);
Texture2D SceneTexture : register(t1);

SamplerState PointSampler  : register(s0);

struct PS_In_Quad
{
    float4 vPosition    : SV_POSITION;
    float2 vTexCoord    : TEXCOORD0;
    float2 vPosition2   : TEXCOORD1;
};

float GetLinearDepth(float fNonLinearDepth)
{
    float fPercFar = CameraFarClip / (CameraFarClip - CameraNearClip);
    return (-CameraNearClip * fPercFar) / (fNonLinearDepth - fPercFar);
}

float GetBlurFactor(float fLinearDepth)
{
    float fDistFromFocalDepth = fLinearDepth - FocalDistance;

    float fBlurAmmount = max(abs(fDistFromFocalDepth) - FocalFalloffNear, 0.0f) /
                         (FocalFalloffFar - FocalFalloffNear);

    return smoothstep(0.0f, 1.0f, fBlurAmmount);
}

float4 PS_DoF(PS_In_Quad input) : SV_TARGET0
{
    // Start with center sample color
    float4 vColorSum = SceneTexture.SampleLevel(PointSampler, input.vTexCoord, 0);
    float fTotalContribution = 1.0f;

    // Depth and blurriness values for center sample
    float fCenterDepth = GetLinearDepth(DepthTexture.SampleLevel(PointSampler, input.vTexCoord, 0).x);
    float fCenterBlur = GetBlurFactor(fCenterDepth);

    [branch]
    if (fCenterBlur > 0.0f)
    {
        // Compute CoC size based on blurriness
        float2 fSizeCoC = (fCenterBlur * CircleOfConfusionScale) * InvSceneResolution;

        // Run through all filter taps
        for (int i = 0; i < DOF_SAMPLE_COUNT; i++)
        {
            // Compute sample coordinates
            float2 vTapCoord = input.vTexCoord + (SampleDirections[i].xy * fSizeCoC);

            // Fetch filter tap sample
            float4 vTapColor = SceneTexture.SampleLevel(PointSampler, vTapCoord, 0);
            float fTapDepth = GetLinearDepth(DepthTexture.SampleLevel(PointSampler, vTapCoord, 0).x);
            float fTapBlur = GetBlurFactor(fTapDepth);

            // Compute tap contribution based on depth and blurriness
            float fTapContribution = (fTapDepth > fCenterDepth) ? 1.0f : fTapBlur;

            // Accumulate color and sample contribution
            vColorSum += vTapColor * fTapContribution;
            fTotalContribution += fTapContribution;
        }
    }

    // Normalize color sum
    return vColorSum / fTotalContribution;
}
