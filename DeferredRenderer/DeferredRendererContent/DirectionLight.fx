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

float2 ShadowMapSize;
texture ShadowMap;
sampler2D ShadowMapSampler = sampler_state
{
    Texture = <ShadowMap>;
    MinFilter = Point; 
    MagFilter = Point; 
    MipFilter = Point;
};

float4x4 InverseViewProjection;

float3 CameraPosition; 

float3 LightDirection;

float3 LightColor;
float LightIntensity;

static const float BIAS = 0.006f;

static const int MAX_CASCADES = 9;
int SqrtCascadeCount = 2;
float4x4 LightViewProjection[MAX_CASCADES];
float2 ClipPlanes[MAX_CASCADES];

float NearClip;
float FarClip;

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
	float4 vPosition2 : TEXCOORD1;
};

struct PS_In_DirectionLight
{
    float4 vPosition : POSITION0;
    float2 vTexCoord : TEXCOORD0;
	float4 vPosition2 : TEXCOORD1;
};

VS_Out_DirectionLight VS_DirectionLight(VS_In_DirectionLight input)
{
    VS_Out_DirectionLight output;

    output.vPosition = float4(input.vPosition, 1.0f);
	output.vPosition2 = output.vPosition;
    output.vTexCoord = input.vTexCoord - ScreenSpaceOffset;

    return output;
}

float4 GetPositionWS(float2 vPositionCS, float fDepth)
{
	float4 vPositionWS = mul(float4(vPositionCS.x,vPositionCS.y, fDepth, 1.0f), InverseViewProjection);
	vPositionWS.xyz = vPositionWS.xyz / vPositionWS.www;
	vPositionWS.w = 1.0f;

	return vPositionWS;
}

float4 PS_DirectionalLightCommon(PS_In_DirectionLight input, float4 vPositionWS)
{
	float4 vColorData = tex2D(RT0Sampler, input.vTexCoord);
    float4 vNormalData = tex2D(RT1Sampler, input.vTexCoord);
    
	float fSpecularIntensity = vColorData.a;
	float fSpecularPower = vNormalData.a * 255.0f;	
		    
	float3 N = vNormalData.xyz * 2.0f - 1.0f;
    float3 L = normalize(LightDirection);
	float3 V = normalize(CameraPosition - vPositionWS);

    float3 R = normalize(V - 2 * dot(N, V) * N);

	float fDiffuseTerm = saturate(dot(N, L));
	float fSpecularTerm = fSpecularIntensity * pow(saturate(dot(R, L)), fSpecularPower);
	
    return LightIntensity * float4(fDiffuseTerm * LightColor, fSpecularTerm);
}

float PCF_Filter(float2 vShadowTexCoord, float fLightDepth, int iSqrtSamples)
{
	float fShadowTerm = 0.0f;  
		
	float radius = (iSqrtSamples - 1) / 2;
	float fWeightAccum = 0.0f;	

	for (int y = -radius; y <= radius; y++)
	{
		for (int x = -radius; x <= radius; x++)
		{
			float2 vOffset = float2(x / ShadowMapSize.x, y / ShadowMapSize.y);
			float2 vSamplePoint = vShadowTexCoord + vOffset;			
			float fDepth = tex2D(ShadowMapSampler, vSamplePoint).x;
			float fSample = (fLightDepth <= fDepth + BIAS);
			
			// Edge tap smoothing
			float xWeight = 1;
			float yWeight = 1;
			
			if (x == -radius)
			{
				xWeight = 1 - frac(vShadowTexCoord.x * ShadowMapSize.x);
			}
			else if (x == radius)
			{
				xWeight = frac(vShadowTexCoord.x * ShadowMapSize.x);
			}
				
			if (y == -radius)
			{
				yWeight = 1 - frac(vShadowTexCoord.y * ShadowMapSize.y);
			}
			else if (y == radius)
			{
				yWeight = frac(vShadowTexCoord.y * ShadowMapSize.y);
			}
				
			fShadowTerm += fSample * xWeight * yWeight;
			fWeightAccum += xWeight * yWeight;
		}											
	}		
	
	fShadowTerm /= fWeightAccum;
	
	return fShadowTerm;
}

float4 PS_DirectionLightShadowed(PS_In_DirectionLight input) : COLOR0
{
	float fDepth = tex2D(RT3Sampler, input.vTexCoord).r;
	float4 vPositionWS = GetPositionWS(input.vPosition2, fDepth);

	float fPercFar = FarClip / (FarClip - NearClip);
	float fSceneZ = ( -NearClip * fPercFar ) / ( fDepth - fPercFar);	

	float3 vSplitColors [4];
	vSplitColors[0] = float3(1, 0, 0);
	vSplitColors[1] = float3(0, 1, 0);
	vSplitColors[2] = float3(0, 0, 1);
	vSplitColors[3] = float3(1, 1, 0);
	int iCurrentSplit = 0;

	// Figure out which split this pixel belongs to, based on view-space depth.
	
	for (int i = 1; i < SqrtCascadeCount * SqrtCascadeCount; i++)
	{
		if (fSceneZ >= ClipPlanes[i].x && fSceneZ < ClipPlanes[i].y)
		{
			iCurrentSplit = i;
		}
	}
	
	float3 vColor = vSplitColors[iCurrentSplit];
	float4x4 mLightViewProj = LightViewProjection[iCurrentSplit];

	float2 vOffset = float2((iCurrentSplit % SqrtCascadeCount) / (float)SqrtCascadeCount,
							(iCurrentSplit / SqrtCascadeCount) / (float)SqrtCascadeCount);

	float4 vLightPosition = mul(vPositionWS, mLightViewProj);
	float fLightDepth = vLightPosition.z / vLightPosition.w;

	float2 vShadowTexCoord = 0.5 * (vLightPosition.xy / vLightPosition.w) + float2(0.5, 0.5);
	vShadowTexCoord.y = 1.0f - vShadowTexCoord.y;
	vShadowTexCoord = (vShadowTexCoord / SqrtCascadeCount) + vOffset + (1.0f / ShadowMapSize);

	float fOccluderDepth = tex2D(ShadowMapSampler, vShadowTexCoord).x;
	//float fShadowed = fLightDepth <= fOccluderDepth + BIAS;
	float fShadowed = PCF_Filter(vShadowTexCoord, fLightDepth, 9);
	
	float4 vLightingColor = PS_DirectionalLightCommon(input, vPositionWS);

	//return float4(vOffset.x, vOffset.x, vOffset.x, 1.0f);
	return fShadowed * float4(vLightingColor.rgb, vLightingColor.a);
}

float4 PS_DirectionLightUnshadowed(PS_In_DirectionLight input) : COLOR0
{
	float fDepth = tex2D(RT3Sampler, input.vTexCoord).r;
	float4 vPositionWS = GetPositionWS(input.vPosition2, fDepth);

	return PS_DirectionalLightCommon(input, vPositionWS);
}

technique DirectionLightUnshadowed
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 VS_DirectionLight();
        PixelShader = compile ps_3_0 PS_DirectionLightUnshadowed();
    }
}

technique DirectionLightShadowed
{
    pass Pass1
    {
        VertexShader = compile vs_3_0 VS_DirectionLight();
        PixelShader = compile ps_3_0 PS_DirectionLightShadowed();
    }
}