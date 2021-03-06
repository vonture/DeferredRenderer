cbuffer cbParticleSystemProperties : register(c0)
{
    float4x4 ViewProjection;
    float4x4 InverseView;
    float4x4 PrevViewProjection;
    float4x4 PrevInverseView;
}

cbuffer cbParticleCameraProperties : register(c1)
{
    float CameraNearClip;
    float CameraFarClip;
    float FadeDistance;
    float Padding0;
    float3 CameraForward;
    float Padding1;
    float3 CameraUp;
    float Padding2;
    float3 CameraRight;
    float Padding3;
}

static const float TwoPi = 6.28318531f;
static const float PiOver4 = 0.785398163;
static const float QuadPositions[4] =
{
    TwoPi * 0.25f + PiOver4,
    TwoPi * 0.00f + PiOver4,
    TwoPi * 0.50f + PiOver4,
    TwoPi * 0.75f + PiOver4,
};

static const float2 QuadTexcoords[4] = 
{ 
    float2(0, 0), 
    float2(1, 0),
    float2(0, 1),
    float2(1, 1),
};

Texture2D DiffuseMap    : register(t0);
Texture2D NormalMap     : register(t1);

Texture2D SceneDepth    : register(t2);

SamplerState ParticleSampler     : register(s0);
SamplerState SceneSampler        : register(s1);

struct VS_In_Particle
{
    float3 vPositionWS        : POSITION;
    float3 vPrevPositionWS    : PREVPOSITION;
    float4 vColor             : COLOR;
    float  fRadius            : RADIUS;
    float  fPrevRadius        : PREVRADIUS;
    float  fRotation          : ROTATION;
    float  fPrevRotation      : PREVROTATION;
};

struct VS_Out_Particle
{
    float3 vPositionWS        : POSITION;
    float3 vPrevPositionWS    : PREVPOSITION;
    float4 vColor             : COLOR;
    float  fRadius            : RADIUS;
    float  fPrevRadius        : PREVRADIUS;
    float  fRotation          : ROTATION;
    float  fPrevRotation      : PREVROTATION;
};

struct PS_In_Particle
{
    float4 vPositionCS      : SV_POSITION;
    float4 vPositionCS2     : POSITION;
    float4 vPrevPositionCS  : PREVPOSITION;
    float4 vColor           : COLOR;
    float2 vTexCoord        : TEXCOORD;
};

struct PS_Out_Particle
{
    float4 vColor        : SV_TARGET0;
    float4 vNormal       : SV_TARGET1;
    float4 vVelocity    : SV_TARGET2;
};

VS_Out_Particle VS_Particle(VS_In_Particle input)
{
    VS_Out_Particle output;

    output.vPositionWS = input.vPositionWS;
    output.vPrevPositionWS = input.vPrevPositionWS;
    output.vColor = input.vColor;
    output.fRadius = input.fRadius;
    output.fPrevRadius = input.fPrevRadius;
    output.fRotation = input.fRotation;
    output.fPrevRotation = input.fPrevRotation;

    return output;
}

[maxvertexcount(4)]
void GS_Particle(point VS_Out_Particle input[1], inout TriangleStream<PS_In_Particle> SpriteStream)
{
    PS_In_Particle output;

    output.vColor = input[0].vColor;

    // Emit two new triangles
    [unroll] 
    for(uint i = 0; i < 4; i++)
    {
        // Calculate current position
        float fAngle = QuadPositions[i] + input[0].fRotation;
        float3 vCornerPosition = float3(cos(fAngle) * input[0].fRadius * 2.0f, sin(fAngle) * input[0].fRadius * 2.0f, 0.0f);

        vCornerPosition = mul(vCornerPosition, (float3x3)InverseView) + input[0].vPositionWS;

        output.vPositionCS = mul(float4(vCornerPosition, 1.0f), ViewProjection);
        output.vPositionCS2 = output.vPositionCS;

        // Calculate previous position
        float fPrevAngle = QuadPositions[i] + input[0].fPrevRotation;
        float3 vPrevCornerPosition = float3(cos(fPrevAngle) * input[0].fPrevRadius * 2.0f, sin(fPrevAngle) * input[0].fPrevRadius * 2.0f, 0.0f);
        vPrevCornerPosition = mul(vPrevCornerPosition, (float3x3)PrevInverseView) + input[0].vPrevPositionWS;

        output.vPrevPositionCS = mul(float4(vPrevCornerPosition, 1.0f), PrevViewProjection);

        // Copy texcoord
        output.vTexCoord = QuadTexcoords[i];

        SpriteStream.Append(output);
    }
    SpriteStream.RestartStrip();
}

float GetLinearDepth(float nonLinearDepth, float nearClip, float farClip)
{
    float fPercFar = farClip / (farClip - nearClip);
    return (-nearClip * fPercFar) / ( nonLinearDepth - fPercFar);
}

float2 GetScreenTexCoord(float2 vPositionCS)
{
    return (float2(vPositionCS.x, -vPositionCS.y) + 1.0f) * 0.5f;
}

PS_Out_Particle PS_Particle(PS_In_Particle input)
{
    PS_Out_Particle output;

    float2 vScreenCoord = GetScreenTexCoord(input.vPositionCS2.xy / input.vPositionCS2.ww);

    float fSceneDepth = GetLinearDepth(SceneDepth.SampleLevel(SceneSampler, vScreenCoord, 0).x,
                                        CameraNearClip, CameraFarClip);

    float fDepth = input.vPositionCS2.z / input.vPositionCS2.w;
    float fParticleDepth = GetLinearDepth(fDepth, CameraNearClip, CameraFarClip);

    float fDepthDiff = fSceneDepth - fParticleDepth;

    float4 vDiffuse = DiffuseMap.Sample(ParticleSampler, input.vTexCoord);
    float4 vNormal = NormalMap.Sample(ParticleSampler, input.vTexCoord);
    float3 vNormalTS = (vNormal.xyz * 2.0f) - 1.0f;

    float3x3 mTangentToWorld = float3x3(-CameraRight, -CameraUp, -CameraForward);
    float3 vNormalWS = normalize(mul(vNormalTS, mTangentToWorld));

    float fDepthFade = smoothstep(0.0f, 1.0f, saturate(fDepthDiff / FadeDistance));
    float fAlpha = input.vColor.a * vDiffuse.a * vNormal.a * fDepthFade;

    float2 vVelocity = input.vPositionCS2.xy - input.vPrevPositionCS.xy;

    output.vColor = float4(vDiffuse.rgb * input.vColor.rgb, fAlpha);
    output.vNormal = float4(vNormalWS, fAlpha);
    output.vVelocity = float4(vVelocity * fAlpha, fDepth, fAlpha);

    return output;
}
