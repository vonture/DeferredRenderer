texture RT0;
sampler RT0Sampler = sampler_state
{
    Texture = (RT0);

    AddressU = Clamp;
    AddressV = Clamp;

    MagFilter = Point;
    MinFilter = Point;
    Mipfilter = Point;
};

texture RT1;
sampler RT1Sampler = sampler_state
{
    Texture = (RT1);

    AddressU = Clamp;
    AddressV = Clamp;

    MagFilter = Point;
    MinFilter = Point;
    Mipfilter = Point;
};

texture RT2;
sampler RT2Sampler = sampler_state
{
    Texture = (RT2);

    AddressU = Clamp;
    AddressV = Clamp;

    MinFilter = Point; 
    MagFilter = Point; 
    MipFilter = Point;
};

texture RT3;
sampler RT3Sampler = sampler_state
{
    Texture = (RT3);

    AddressU = Clamp;
    AddressV = Clamp;

    MagFilter = Point;
    MinFilter = Point;
    Mipfilter = Point;
};

float4x4 InverseViewProjection; 

float3 CameraPosition; 

float3 LightDirection;

float3 LightColor;

float2 ScreenSpaceOffset;

struct VS_In_DirectionLight
{
    float3 vPosition : POSITION0;
    float2 vTexCoord : TEXCOORD0;
};

struct VS_Out_DirectionLight
{
    float4 vPosition : POSITION0;
    float2 vTexCoord : TEXCOORD0;
};

struct PS_In_DirectionLight
{
    float4 vPosition : POSITION0;
    float2 vTexCoord : TEXCOORD0;
};

VS_Out_DirectionLight VS_DirectionLight(VS_In_DirectionLight input)
{
    VS_Out_DirectionLight output;

    output.vPosition = float4(input.vPosition, 1.0f);
    output.vTexCoord = input.vTexCoord - ScreenSpaceOffset;

    return output;
}

float4 PS_DirectionLight(PS_In_DirectionLight input) : COLOR0
{
	float4 vColorData = tex2D(RT0Sampler, input.vTexCoord);
    float4 vNormalData = tex2D(RT1Sampler, input.vTexCoord);
    
	float fSpecularIntensity = vColorData.a;
	float fSpecularPower = vNormalData.a * 255.0f;	

    float fDepth = tex2D(RT2Sampler, input.vTexCoord).r;

    //compute screen-space position
    float4 vPositionWS;
    vPositionWS.x = input.vTexCoord.x * 2.0f - 1.0f;
    vPositionWS.y = -input.vTexCoord.y * 2.0f - 1.0f;
    vPositionWS.z = fDepth;
    vPositionWS.w = 1.0f;
    //transform to world space
    vPositionWS = mul(vPositionWS, InverseViewProjection);
    vPositionWS /= vPositionWS.w;
    
	float3 N = vNormalData.xyz * 2.0f - 1.0f;
    float3 L = normalize(LightDirection);
	float3 V = normalize(CameraPosition - vPositionWS);

    float3 R = normalize(V - 2 * dot(N, V) * N);

	float fDiffuseTerm = saturate(dot(N, L));
	float fSpecularTerm = fSpecularIntensity * pow(saturate(dot(R, L)), fSpecularPower);
	
    return float4(fDiffuseTerm * LightColor, fSpecularTerm);
}

technique DirectionLight
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_DirectionLight();
        PixelShader = compile ps_2_0 PS_DirectionLight();
    }
}