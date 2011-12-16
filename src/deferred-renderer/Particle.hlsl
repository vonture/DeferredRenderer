cbuffer cbParticleSystemProperties : register(c0)
{
	float4x4 ViewProjection	: packoffset(c0.x);
	float4x4 InverseView	: packoffset(c4.x);
}

static const float3 QuadPositions[4] =
{
    float3(-1,  1, 0 ),
    float3( 1,  1, 0 ),
    float3(-1, -1, 0 ),
    float3( 1, -1, 0 ),
};

static const float2 QuadTexcoords[4] = 
{ 
    float2(0,0), 
    float2(1,0),
    float2(0,1),
    float2(1,1),
};

Texture2D DiffuseMap	: register(t0);
Texture2D NormalMap		: register(t1);

SamplerState Sampler	: register(s0);

struct VS_In_Particle
{
	float3 vPositionOS	: POSITION;
	float4 vColor		: COLOR;
	float  fRadius		: RADIUS;
	float  fRotation	: ROTATION;
};

struct VS_Out_Particle
{
	float3 vPositionOS	: POSITION;
	float4 vColor		: COLOR;
	float  fRadius		: RADIUS;
	float  fRotation	: ROTATION;
};

struct PS_In_Particle
{
	float4 vPositionCS	: SV_POSITION;
	float4 vColor		: COLOR;
	float2 vTexCoord	: TEXCOORD;
};

struct PS_Out_Particle
{
	float4 vColor	: SV_TARGET0;
};

VS_Out_Particle VS_Particle(VS_In_Particle input)
{
	VS_Out_Particle output;

	output.vPositionOS = input.vPositionOS;
	output.vColor = input.vColor;
	output.fRadius = input.fRadius;
	output.fRotation = input.fRotation;

	return output;
}

[maxvertexcount(4)]
void GS_Particle(point VS_Out_Particle input[1], inout TriangleStream<PS_In_Particle> SpriteStream)
{
	PS_In_Particle output;
    
    output.vColor = input[0].vColor;
    
	float3 vRotationVector = float3(cos(input[0].fRotation), -sin(input[0].fRotation), 1.0f);

    // Emit two new triangles
    [unroll] 
	for(uint i = 0; i < 4; i++)
    {
        float3 vCornerPosition = (QuadPositions[i] * vRotationVector) * input[0].fRadius;
        vCornerPosition = mul(vCornerPosition, (float3x3)InverseView) + input[0].vPositionOS;

        output.vPositionCS = mul(float4(vCornerPosition, 1.0f), ViewProjection);
        output.vTexCoord = QuadTexcoords[i];
            
        SpriteStream.Append(output);
    }
    SpriteStream.RestartStrip();
}

PS_Out_Particle PS_Particle(PS_In_Particle input)
{
	PS_Out_Particle output;

	float4 vDiffuse = DiffuseMap.Sample(Sampler, input.vTexCoord);
	float3 vNormal = (NormalMap.Sample(Sampler, input.vTexCoord).xyz * 2.0f) - 1.0f;

	output.vColor = float4(input.vColor.rgb * input.vColor.a, 0.0f);

	return output;
}