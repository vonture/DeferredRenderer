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

texture LightMap;
sampler LightMapSampler = sampler_state
{
    Texture = (LightMap);

    AddressU = Clamp;
    AddressV = Clamp;

    MagFilter = Point;
    MinFilter = Point;
    Mipfilter = Point;
};

float2 ScreenSpaceOffset;

struct VS_In_Combine
{
    float3 vPosition : POSITION0;
    float2 vTexCoord : TEXCOORD0;
};

struct VS_Out_Combine
{
    float4 vPosition : POSITION0;
    float2 vTexCoord : TEXCOORD0;
};

struct PS_In_Combine
{
    float4 vPosition : POSITION0;
    float2 vTexCoord : TEXCOORD0;
};


VS_Out_Combine VS_Combine(VS_In_Combine input)
{
    VS_Out_Combine output;
    output.vPosition = float4(input.vPosition,1);
    output.vTexCoord = input.vTexCoord - ScreenSpaceOffset;
    return output;
}

float4 PS_Combine(PS_In_Combine input) : COLOR0
{
	// Diffuse colour and specular intensity
    float4 vColor = tex2D(RT0Sampler, input.vTexCoord);

	// Normals and specular power
	float4 vNormals = tex2D(RT1Sampler, input.vTexCoord);

	// Translucency and ambient occlusion
	float4 vTrans = tex2D(RT2Sampler, input.vTexCoord);

	// Depth
	float fDepth = tex2D(RT3Sampler, input.vTexCoord).r;

	// Light colour and specular
	float4 vLighting = tex2D(LightMapSampler, input.vTexCoord);

	float3 vDiffuse = vColor.rgb;
	float fAmbient = vTrans.a;
	float3 vLightColor = vLighting.rgb;
	float fSpecular = vLighting.a;

	float3 vFinalColour = fAmbient * vDiffuse +
						  vLightColor * vDiffuse +
						  vLightColor * fSpecular;

    return float4(vFinalColour, 1.0f);
	//return float4(vLightColor, 1.0f);
	//return float4(vColor.rgb, 1.0f);
	//return float4(vNormals.a, vNormals.a, vNormals.a, 1.0f);
	//return float4(fSpecular, fSpecular, fSpecular, 1.0f);
	//return float4(fDepth, fDepth, fDepth, 1.0f);
	//return float4(vNormals.rgb, 1.0f);
}

technique GBufferCombine
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Combine();
        PixelShader = compile ps_2_0 PS_Combine();
    }
}
