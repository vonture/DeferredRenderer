#define EPSILON 1e-5

cbuffer cbCameraProperties : register(b0)
{
    float4x4 InverseViewProjection    : packoffset(c0);
    float4   CameraPosition           : packoffset(c4);
}

cbuffer cbModelProperties : register(b1)
{
    float4x4 World                  : packoffset(c0);
    float4x4 WorldViewProjection    : packoffset(c4);
}

cbuffer cbLightProperties : register(b2)
{
    float3 LightPosition    : packoffset(c0);
    float  LightRadius      : packoffset(c0.w);
    float3 LightColor       : packoffset(c1);
    float  LightBrightness  : packoffset(c1.w);
}

cbuffer cbShadowProperties : register(b3)
{
    float2 CameraClips;
    float2 ShadowMapSize;
    float Bias;
    float3 Padding2;
    float4x4 ShadowMatrix;
}

Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture  : register(t1);
Texture2D DepthTexture   : register(t2);

Texture2D ShadowMap : register(t3);

SamplerState SceneSampler            : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

struct VS_In_PointLight
{
    float4 vPositionOS    : POSITION;
};

struct VS_Out_PointLight
{
    float4 vPositionCS     : SV_POSITION;
    float4 vPositionCS2    : TEXCOORD0;
};

VS_Out_PointLight VS_PointLight(VS_In_PointLight input)
{
    VS_Out_PointLight output;

    output.vPositionCS = mul(input.vPositionOS, WorldViewProjection);
    output.vPositionCS2 = output.vPositionCS;

    return output;
}

float4 GetPositionWS(float2 vPositionCS, float fDepth)
{
    float4 vPositionWS = mul(float4(vPositionCS, fDepth, 1.0f), InverseViewProjection);
    vPositionWS.xyz = vPositionWS.xyz / vPositionWS.www;
    vPositionWS.w = 1.0f;

    return vPositionWS;
}

float2 GetScreenTexCoord(float2 vPositionCS)
{
    return (float2(vPositionCS.x, -vPositionCS.y) + 1.0f) * 0.5f;
}

float4 PS_PointLightCommon(VS_Out_PointLight input, float4 vPositionWS, float2 vTexCoord)
{
    float4 vNormalData = NormalTexture.Sample(SceneSampler, vTexCoord);

    float fSpecularIntensity = DiffuseTexture.Sample(SceneSampler, vTexCoord).a;
    float fSpecularPower = vNormalData.a;

    float3 vLightDir = LightPosition - vPositionWS.xyz;

    float fAttenuation = 1.0f - saturate(length(vLightDir) / LightRadius);
    //fAttenuation = fAttenuation * fAttenuation;

    float3 N = vNormalData.xyz;
    float3 L = normalize(vLightDir);
    float3 V = normalize(CameraPosition.xyz - vPositionWS.xyz);

    float3 R = normalize(V - 2 * dot(N, V) * N);

    float fDiffuseTerm = saturate(dot(N, L));
    float fSpecularTerm = fSpecularIntensity * pow(clamp(dot(R, L), EPSILON, 1.0f), fSpecularPower);

    return fAttenuation * float4(fDiffuseTerm * LightColor * LightBrightness, fSpecularTerm);
}

float4 PS_PointLightUnshadowed(VS_Out_PointLight input) : SV_TARGET0
{
    input.vPositionCS2.xyz = input.vPositionCS2.xyz / input.vPositionCS2.www;
    input.vPositionCS2.w = 1.0f;

    float2 vScreenCoord = GetScreenTexCoord(input.vPositionCS2.xy);

    float fDepth = DepthTexture.Sample(SceneSampler, vScreenCoord).r;
    float4 vPositionWS = GetPositionWS(input.vPositionCS2.xy, fDepth);

    return PS_PointLightCommon(input, vPositionWS, vScreenCoord);
}

float SampleShadow(in float2 vShadowTexCoord, in float fSceneDepth)
{
    // Edge tap smoothing
    const int Radius = 2;
    const int NumSamples = (Radius * 2 + 1) * (Radius * 2 + 1);

    float2 fracs = frac(vShadowTexCoord.xy * ShadowMapSize);
    float leftEdge = 1.0f - fracs.x;
    float rightEdge = fracs.x;
    float topEdge = 1.0f - fracs.y;
    float bottomEdge = fracs.y;

    float shadowVisibility = 0.0f;

    float2 invShadowMapSize = 1.0f / ShadowMapSize;

    [unroll(NumSamples)]
    for (int y = -Radius; y <= Radius; y++)
    {
        [unroll(NumSamples)]
        for (int x = -Radius; x <= Radius; x++)
        {
            float2 offset = float2(x, y) * invShadowMapSize;
            float sample = ShadowMap.SampleCmp(ShadowSampler, vShadowTexCoord + offset, fSceneDepth - Bias).x;

            float xWeight = 1;
            float yWeight = 1;

            if(x == -Radius)
                xWeight = leftEdge;
            else if(x == Radius)
                xWeight = rightEdge;

            if(y == -Radius)
                yWeight = topEdge;
            else if(y == Radius)
                yWeight = bottomEdge;

            shadowVisibility += sample * xWeight * yWeight;
        }
    }

    shadowVisibility /= NumSamples;
    shadowVisibility *= 1.5f;

    return shadowVisibility;
}

float4 PS_PointLightShadowed(VS_Out_PointLight input) : SV_TARGET0
{
    input.vPositionCS2.xyz = input.vPositionCS2.xyz / input.vPositionCS2.www;
    input.vPositionCS2.w = 1.0f;

    float2 vScreenCoord = GetScreenTexCoord(input.vPositionCS2.xy);

    float fDepth = DepthTexture.Sample(SceneSampler, vScreenCoord).r;
    float4 vPositionWS = GetPositionWS(input.vPositionCS2.xy, fDepth);

    float4 vPositionLS = mul(vPositionWS, ShadowMatrix);

    float fLength = length(vPositionLS);
    vPositionLS /= fLength;

    float fSceneDepth = (fLength - CameraClips.x) / (CameraClips.y - CameraClips.x);

    float2 vShadowTexCoord;
    if(vPositionLS.z >= 0.0f)
    {
        vShadowTexCoord.x = (vPositionLS.x / (1.0f + vPositionLS.z)) * 0.5f + 0.5f; 
        vShadowTexCoord.y = 1.0f - ((vPositionLS.y / (1.0f + vPositionLS.z)) * 0.5f + 0.5f);

        // Offset to the left side of the shadow map
        vShadowTexCoord.x = vShadowTexCoord.x * 0.5f;
    }
    else
    {
        // for the back the z has to be inverted
        vShadowTexCoord.x =  (vPositionLS.x /  (1.0f - vPositionLS.z)) * 0.5f + 0.5f;
        vShadowTexCoord.y =  1.0f - ((vPositionLS.y /  (1.0f - vPositionLS.z)) * 0.5f + 0.5f);

        // Offset to the right side of the shadow map
        vShadowTexCoord.x = vShadowTexCoord.x * 0.5f + 0.5f;
    }

    float fShadow = SampleShadow(vShadowTexCoord, fSceneDepth);

    return fShadow * PS_PointLightCommon(input, vPositionWS, vScreenCoord);
}
