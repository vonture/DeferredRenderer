float4x4 World;
float4x4 View;
float4x4 Projection;

float4x4 InverseViewProjection; 

float3 CameraPosition; 

float3 LightPosition;
float3 LightColor;
float LightRadius;
float LightIntensity = 1.0f;

float2 ScreenSpaceOffset;

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

struct VS_In_PointLight
{
    float3 vPositionOS : POSITION0;
};

struct VS_Out_PointLight
{
    float4 vPositionCS	: POSITION0;
    float4 vPositionCS2	: TEXCOORD0;
};

struct PS_In_PointLight
{
    float4 vPositionCS	: TEXCOORD0;
};

VS_Out_PointLight VS_PointLight(VS_In_PointLight input)
{
    VS_Out_PointLight output;

	float4 worldPosition = mul(float4(input.vPositionOS, 1.0f), World);
    float4 viewPosition = mul(worldPosition, View);
    output.vPositionCS = mul(viewPosition, Projection);
    output.vPositionCS2 = output.vPositionCS;

    return output;
}

float4 PS_PointLight(PS_In_PointLight input) : COLOR0
{
    input.vPositionCS.xy /= input.vPositionCS.w;
    float2 vScreenCoord = 0.5f * (float2(input.vPositionCS.x, -input.vPositionCS.y) + 1.0f);
	vScreenCoord -= ScreenSpaceOffset;

	float4 vColorData = tex2D(RT0Sampler, vScreenCoord);
    float4 vNormalData = tex2D(RT1Sampler, vScreenCoord);
	float fDepth = tex2D(RT3Sampler, vScreenCoord).r;

	float fSpecularIntensity = vColorData.a;
	float fSpecularPower = vNormalData.a * 255.0f;	
		
     //compute screen-space position
    float4 vPositionWS;
    vPositionWS.xy = input.vPositionCS.xy;
    vPositionWS.z = fDepth;
    vPositionWS.w = 1.0f;
    //transform to world space
    vPositionWS = mul(vPositionWS, InverseViewProjection);
    vPositionWS /= vPositionWS.w;	

    // surface-to-light vector
    float3 vLightDir = LightPosition - vPositionWS;

    // compute attenuation based on distance - linear attenuation
    float fAttenuation = saturate(1.0f - length(vLightDir) / LightRadius); 

	float3 N = vNormalData.xyz * 2.0f - 1.0f;
    float3 L = normalize(vLightDir);
	float3 V = normalize(CameraPosition - vPositionWS);

	float3 R = normalize(V - 2 * dot(N, V) * N);

    float fDiffuseTerm = saturate(dot(N, L));
	float fSpecularTerm = fSpecularIntensity * pow(saturate(dot(R, L)), fSpecularPower);	

    //take into account attenuation and lightIntensity.
    return fAttenuation * LightIntensity * float4(fDiffuseTerm * LightColor, fSpecularTerm);

	//return float4(fAttenuation * LightIntensity * fDiffuseTerm * LightColor, 1.0f);
}

technique Technique1
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_PointLight();
        PixelShader = compile ps_2_0 PS_PointLight();
    }
}
