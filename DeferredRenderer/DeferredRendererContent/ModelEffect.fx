float4x4 World;
float4x4 View;
float4x4 Projection;

texture DiffuseMap;
sampler DiffuseSampler = sampler_state
{
    Texture = (DiffuseMap);

    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
    
    AddressU = Wrap;
    AddressV = Wrap;
};

float SpecularPower = 32.0f;
static const float SPECULAR_MAX = 255.0f;
texture SpecularMap;
sampler SpecularSampler = sampler_state
{
    Texture = (SpecularMap);

    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
    
    AddressU = Wrap;
    AddressV = Wrap;
};

texture NormalMap;
sampler NormalSampler = sampler_state
{
    Texture = (NormalMap);

    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
    
    AddressU = Wrap;
    AddressV = Wrap;
};

float Ambient = 0.15f;
texture AmbientOcclusionMap;
sampler AmbientOcclusionSampler = sampler_state
{
    Texture = (AmbientOcclusionMap);

    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
    
    AddressU = Wrap;
    AddressV = Wrap;
};

texture DisplacementMap;
sampler DisplacementSampler = sampler_state
{
    Texture = (DisplacementMap);

    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
    
    AddressU = Wrap;
    AddressV = Wrap;
};

struct VS_In_Model
{
	float4 vPositionOS	: POSITION;
	float3 vNormalOS	: NORMAL;
	float3 vBinormalOS	: BINORMAL;
	float3 vTangentOS	: TANGENT;
	float2 vTexCoord	: TEXCOORD0;
};

struct VS_Out_Model
{
	float4 vPositionCS	: POSITION;
	float2 vTexCoord	: TEXCOORD0;
	float3 vNormalWS	: TEXCOORD1;
	float3 vBinormalWS	: TEXCOORD2;
	float3 vTangentWS	: TEXCOORD3;
	float2 vDepthCS		: TEXCOORD4;
};

struct PS_In_Model
{
	float2 vTexCoord	: TEXCOORD0;
	float3 vNormalWS	: TEXCOORD1;
	float3 vBinormalWS	: TEXCOORD2;
	float3 vTangentWS	: TEXCOORD3;
	float2 vDepthCS		: TEXCOORD4;
};

struct PS_Out_Model
{    
	float4 RT0 : COLOR0;
	float4 RT1 : COLOR1;
	float4 RT2 : COLOR2;
	float4 RT3 : COLOR3;
};

VS_Out_Model VS_Model(VS_In_Model input)
{
    VS_Out_Model output;

	float4x4 wvp = mul(World, mul(View, Projection));
    output.vPositionCS = mul(input.vPositionOS, wvp);
	output.vDepthCS = output.vPositionCS.zw;

	output.vNormalWS = mul(input.vNormalOS, World);
	output.vBinormalWS = mul(normalize(input.vBinormalOS), World);
	output.vTangentWS = mul(normalize(input.vTangentOS), World);

	output.vTexCoord = input.vTexCoord;

    return output;
}

PS_Out_Model PS_Model(PS_In_Model input)
{
	PS_Out_Model output;

	float4 vDiffuse = tex2D(DiffuseSampler, input.vTexCoord);
	
	float fAmbientTerm = tex2D(AmbientOcclusionSampler, input.vTexCoord).r * Ambient;

	float3 vTexNorm = tex2D(NormalSampler, input.vTexCoord).xyz * 2.0f - 1.0f;
	float3 vNormal = normalize(input.vNormalWS * vTexNorm.b + input.vTangentWS * vTexNorm.g + input.vBinormalWS * vTexNorm.r);
	
	float fSpecularIntensity = tex2D(SpecularSampler, input.vTexCoord).r;	
	float fSpecularPower = SpecularPower / 255.0f;

	float fDepth = input.vDepthCS.x / input.vDepthCS.y;

	output.RT0 = float4(vDiffuse.rgb, fSpecularIntensity);
	output.RT1 = float4(0.5f * (vNormal.xyz + 1.0f), fSpecularPower);
	output.RT2 = float4(0.0f, fAmbientTerm, 0.0f, 0.0f);
	output.RT3 = float4(fDepth, 1.0f, 1.0f, 1.0f);	

	return output;
}

technique ModelEffect
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Model();
        PixelShader = compile ps_2_0 PS_Model();
    }
}
