#define FXAA_PC 1

#ifndef FXAA_HLSL_4
#define FXAA_HLSL_4 0
#endif

#ifndef FXAA_HLSL_5
#define FXAA_HLSL_5 1
#endif

#ifndef FXAA_QUALITY__PRESET
#define FXAA_QUALITY__PRESET 12
#endif

#ifndef FXAA_GREEN_AS_LUMA
#define FXAA_GREEN_AS_LUMA 1
#endif

#define FLOAT_UNUSED    0.0f
#define FLOAT4_UNUSED    float4(0.0f, 0.0f, 0.0f, 0.0f)

#include "Fxaa3_11.hlsl"

cbuffer cbFXAAProperties : register(b0)
{
    float2 InverseSceneSize    : packoffset(c0.x); // FxaaFloat2 fxaaQualityRcpFrame;
    float  Subpixel            : packoffset(c0.z); // FxaaFloat  fxaaQualitySubpix;
    float  EdgeThreshold       : packoffset(c0.w); // FxaaFloat  fxaaQualityEdgeThreshold;
    float  EdgeThresholdMin    : packoffset(c1.x); // FxaaFloat  fxaaQualityEdgeThresholdMin;
    float3 Padding             : packoffset(c1.y);
}

Texture2D SceneTexture      : register(t0);
SamplerState LinearSampler  : register(s0);

struct PS_In_Quad
{
    float4 vPosition    : SV_POSITION;
    float2 vTexCoord    : TEXCOORD0;
    float2 vPosition2   : TEXCOORD1;
};

float4 PS_FXAA(PS_In_Quad input) : SV_TARGET0
{
    FxaaTex sceneTex;
    sceneTex.smpl = LinearSampler;
    sceneTex.tex = SceneTexture;

    return FxaaPixelShader
    (
        input.vTexCoord,    // FxaaFloat2 pos,
        FLOAT4_UNUSED,      // FxaaFloat4 fxaaConsolePosPos,
        sceneTex,           // FxaaTex tex,
        sceneTex,           // FxaaTex fxaaConsole360TexExpBiasNegOne,
        sceneTex,           // FxaaTex fxaaConsole360TexExpBiasNegTwo,
        InverseSceneSize,   // FxaaFloat2 fxaaQualityRcpFrame,
        FLOAT4_UNUSED,      // FxaaFloat4 fxaaConsoleRcpFrameOpt,
        FLOAT4_UNUSED,      // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
        FLOAT4_UNUSED,      // FxaaFloat4 fxaaConsole360RcpFrameOpt2,
        Subpixel,           // FxaaFloat fxaaQualitySubpix,
        EdgeThreshold,      // FxaaFloat fxaaQualityEdgeThreshold,
        EdgeThresholdMin,   // FxaaFloat fxaaQualityEdgeThresholdMin,
        FLOAT_UNUSED,       // FxaaFloat fxaaConsoleEdgeSharpness,
        FLOAT_UNUSED,       // FxaaFloat fxaaConsoleEdgeThreshold,
        FLOAT_UNUSED,       // FxaaFloat fxaaConsoleEdgeThresholdMin,
        FLOAT4_UNUSED       // FxaaFloat4 fxaaConsole360ConstDir
    );
}
