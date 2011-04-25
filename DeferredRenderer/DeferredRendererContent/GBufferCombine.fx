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

	// Light colour and specular
	float4 vLighting = tex2D(LightMapSampler, input.vTexCoord);

	float3 vDiffuse = vColor.rgb;
	float fAmbient = vTrans.g;
	float3 vLightColor = vLighting.rgb;
	float fSpecular = vLighting.a;

	float3 vFinalColour = fAmbient * vDiffuse +
						  vLightColor * vDiffuse +
						  vLightColor * fSpecular;

    return float4(vFinalColour, 1.0f);
}

float4 PS_DiffuseColor(PS_In_Combine input) : COLOR0
{
	float4 vColor = tex2D(RT0Sampler, input.vTexCoord);

	return float4(vColor.rgb, 1.0f);
}

float4 PS_SpecularIntensity(PS_In_Combine input) : COLOR0
{
	float fSpecularIntensity = tex2D(RT0Sampler, input.vTexCoord).a;

	return float4(fSpecularIntensity, fSpecularIntensity, fSpecularIntensity, 1.0f);
}

float4 PS_Normals(PS_In_Combine input) : COLOR0
{
	float4 vNormals = tex2D(RT1Sampler, input.vTexCoord);

	return float4(vNormals.xyz, 1.0f);
}

float4 PS_SpecularPower(PS_In_Combine input) : COLOR0
{
	float fSpecularPower = tex2D(RT1Sampler, input.vTexCoord).a;

	return float4(fSpecularPower, fSpecularPower, fSpecularPower, 1.0f);
}

float4 PS_AmbientOcclusion(PS_In_Combine input) : COLOR0
{
	float fAmbient = tex2D(RT2Sampler, input.vTexCoord).g;

	return float4(fAmbient, fAmbient, fAmbient, 1.0f);
}

float4 PS_Translucency(PS_In_Combine input) : COLOR0
{
	float fTranslucency = tex2D(LightMapSampler, input.vTexCoord).b;

	return float4(fTranslucency, fTranslucency, fTranslucency, 1.0f);
}

float4 PS_Depth(PS_In_Combine input) : COLOR0
{
	float fDepth = tex2D(RT3Sampler, input.vTexCoord).r;

	return float4(fDepth, fDepth, fDepth, 1.0f);
}

float4 PS_LightColor(PS_In_Combine input) : COLOR0
{
	float4 vColor = tex2D(LightMapSampler, input.vTexCoord);

	return float4(vColor.rgb, 1.0f);
}

float4 PS_LightSpecular(PS_In_Combine input) : COLOR0
{
	float fLightSpecular = tex2D(LightMapSampler, input.vTexCoord).a;

	return float4(fLightSpecular, fLightSpecular, fLightSpecular, 1.0f);
}

technique Final
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Combine();
        PixelShader = compile ps_2_0 PS_Combine();
    }
}

technique DiffuseColor
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Combine();
        PixelShader = compile ps_2_0 PS_DiffuseColor();
    }
}

technique SpecularIntensity
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Combine();
        PixelShader = compile ps_2_0 PS_SpecularIntensity();
    }
}

technique Normals
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Combine();
        PixelShader = compile ps_2_0 PS_Normals();
    }
}

technique SpecularPower
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Combine();
        PixelShader = compile ps_2_0 PS_SpecularPower();
    }
}

technique AmbientOcclusion
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Combine();
        PixelShader = compile ps_2_0 PS_AmbientOcclusion();
    }
}

technique Translucency
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Combine();
        PixelShader = compile ps_2_0 PS_Translucency();
    }
}

technique Depth
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Combine();
        PixelShader = compile ps_2_0 PS_Depth();
    }
}

technique LightColor
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Combine();
        PixelShader = compile ps_2_0 PS_LightColor();
    }
}

technique LightSpecular
{
    pass Pass1
    {
        VertexShader = compile vs_2_0 VS_Combine();
        PixelShader = compile ps_2_0 PS_LightSpecular();
    }
}