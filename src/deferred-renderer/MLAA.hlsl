#ifndef MAX_DISTANCE
#define MAX_DISTANCE 16
#endif

#define EPSILON 1e-5

#ifndef EDGE_DETECT_DEPTH
#define EDGE_DETECT_DEPTH 1
#endif

#ifndef EDGE_DETECT_NORMAL
#define EDGE_DETECT_NORMAL 1
#endif

#ifndef EDGE_DETECT_COLOR
#define EDGE_DETECT_COLOR 0
#endif

#define GREY float3(0.212671f, 0.715160f, 0.072169f)

cbuffer cbMLAAProperties : register(cb0)
{
    float2 InverseSceneSize     : packoffset(c0.x);
    float DepthThreshold        : packoffset(c0.z);
    float NormalThreshold       : packoffset(c0.w);
    float LuminanceThreshold    : packoffset(c1.x);
    float CameraNearClip        : packoffset(c1.y);
    float CameraFarClip         : packoffset(c1.z);
    int MaxSearchSteps          : packoffset(c1.w);
}

Texture2D Texture0 : register(t0);
Texture2D Texture1 : register(t1);
Texture2D Texture2 : register(t2);

SamplerState PointSampler  : register(s0);
SamplerState LinearSampler : register(s1);

struct PS_In_Quad
{
    float4 vPosition    : SV_POSITION;
    float2 vTexCoord    : TEXCOORD0;
    float2 vPosition2   : TEXCOORD1;
};

float GetLinearDepth(float nonLinearDepth, float nearClip, float farClip)
{
    float fPercFar = farClip / (farClip - nearClip);
    return ( -nearClip * fPercFar ) / ( nonLinearDepth - fPercFar);
}

float CalcLuminance(float3 color)
{
    return max(dot(color, GREY), EPSILON);
}

float4 PS_EdgeDetect(PS_In_Quad input) : SV_TARGET0
{
    float4 edges = 0;

#if EDGE_DETECT_DEPTH
    float D =       GetLinearDepth(Texture0.SampleLevel(PointSampler, input.vTexCoord, 0).x,              CameraNearClip, CameraFarClip);
    float Dleft =   GetLinearDepth(Texture0.SampleLevel(PointSampler, input.vTexCoord, 0, int2(-1, 0)).x, CameraNearClip, CameraFarClip);
    float Dtop =    GetLinearDepth(Texture0.SampleLevel(PointSampler, input.vTexCoord, 0, int2(0, -1)).x, CameraNearClip, CameraFarClip);
    float Dright =  GetLinearDepth(Texture0.SampleLevel(PointSampler, input.vTexCoord, 0, int2(1, 0)).x,  CameraNearClip, CameraFarClip);
    float Dbottom = GetLinearDepth(Texture0.SampleLevel(PointSampler, input.vTexCoord, 0, int2(0, 1)).x,  CameraNearClip, CameraFarClip);

    float4 deltaDepth = abs(D.xxxx - float4(Dleft, Dtop, Dright, Dbottom));
    float4 edgesDepth = step(DepthThreshold.xxxx, deltaDepth);

    edges += edgesDepth;
#endif

#if EDGE_DETECT_NORMAL
    float3 N =          Texture1.SampleLevel(PointSampler, input.vTexCoord, 0              ).xyz;
    float3 Nleft =      Texture1.SampleLevel(PointSampler, input.vTexCoord, 0, int2(-1,  0)).xyz;
    float3 Ntop =       Texture1.SampleLevel(PointSampler, input.vTexCoord, 0, int2( 0, -1)).xyz;
    float3 Nright =     Texture1.SampleLevel(PointSampler, input.vTexCoord, 0, int2( 1,  0)).xyz;
    float3 Nbottom =    Texture1.SampleLevel(PointSampler, input.vTexCoord, 0, int2( 0,  1)).xyz;

    float4 deltaNorm = 1.0f - float4(dot(N, Nleft), dot(N, Ntop), dot(N, Nright), dot(N, Nbottom));
    float4 edgesNormal = step(NormalThreshold.xxxx, deltaNorm);

    edges += edgesNormal;
#endif

#if EDGE_DETECT_COLOR
    float L =       CalcLuminance(Texture2.SampleLevel(PointSampler, input.vTexCoord, 0).rgb);
    float Lleft =   CalcLuminance(Texture2.SampleLevel(PointSampler, input.vTexCoord, 0, int2(-1, 0)).rgb);
    float Ltop =    CalcLuminance(Texture2.SampleLevel(PointSampler, input.vTexCoord, 0, int2(0, -1)).rgb);
    float Lright =  CalcLuminance(Texture2.SampleLevel(PointSampler, input.vTexCoord, 0, int2(1, 0)).rgb);
    float Lbottom = CalcLuminance(Texture2.SampleLevel(PointSampler, input.vTexCoord, 0, int2(0, 1)).rgb);

    float4 deltaLum = abs(L.xxxx - float4(Lleft, Ltop, Lright, Lbottom));
    float4 edgesLum = step(LuminanceThreshold.xxxx, deltaLum);

    edges += edgesLum;
#endif

    if (abs(dot(edges, 1.0f)) < EPSILON)
    {
        discard;
    }

    return edges;
}

float SearchXLeft(float2 texcoord) 
{
    // offset by 0.5 to sample between edgels, thus fetching two in a row
    float2 sampleCoord = texcoord - (float2(1.5f, 0.0f) * InverseSceneSize);

    float edge = 0.0f;

    int index;
    for (index = 0; index < MaxSearchSteps; index++) 
    {
        edge = Texture0.SampleLevel(LinearSampler, sampleCoord, 0).g;

        // We compare with 0.9 to prevent bilinear access precision problems
        [flatten]
        if (edge < 0.9f)
        {
            break;
        }

        sampleCoord -= float2(2.0f, 0.0f) * InverseSceneSize;
    }

    // When we exit the loop without founding the end, we want to return -2 * maxSearchSteps
    return max(-2.0f * index - 2.0f * edge, -2.0f * MaxSearchSteps);
}

float SearchXRight(float2 texcoord) 
{
    // offset by 0.5 to sample between edgels, thus fetching two in a row
    float2 sampleCoord = texcoord + (float2(1.5f, 0.0f) * InverseSceneSize);

    float edge = 0.0f;

    int index;
    for (index = 0; index < MaxSearchSteps; index++) 
    {
        edge = Texture0.SampleLevel(LinearSampler, sampleCoord, 0).g;

        // We compare with 0.9 to prevent bilinear access precision problems
        [flatten]
        if (edge < 0.9f)
        {
            break;
        }

        sampleCoord += float2(2.0f, 0.0f) * InverseSceneSize;
    }

    return min(2.0f * index + 2.0f * edge, 2.0f * MaxSearchSteps);
}

float SearchYUp(float2 texcoord) 
{
    // offset by 0.5 to sample between edgels, thus fetching two in a row
    float2 sampleCoord = texcoord - (float2(0.0f, 1.5f) * InverseSceneSize);

    float edge = 0.0f;

    int index;
    for (index = 0; index < MaxSearchSteps; index++) 
    {
        edge = Texture0.SampleLevel(LinearSampler, sampleCoord, 0).r;

        // We compare with 0.9 to prevent bilinear access precision problems
        [flatten]
        if (edge < 0.9f)
        {
            break;
        }

        sampleCoord -= float2(0.0f, 2.0f) * InverseSceneSize;
    }

    return max(-2.0f * index - 2.0f * edge, -2.0f * MaxSearchSteps);
}

float SearchYDown(float2 texcoord) 
{
    // offset by 0.5 to sample between edgels, thus fetching two in a row
    float2 sampleCoord = texcoord + (float2(0.0f, 1.5f) * InverseSceneSize);

    float edge = 0.0f;

    int index;
    for (index = 0; index < MaxSearchSteps; index++) 
    {
        edge = Texture0.SampleLevel(LinearSampler, sampleCoord, 0).r;

        // We compare with 0.9 to prevent bilinear access precision problems
        [flatten]
        if (edge < 0.9f)
        {
                break;
        }

        sampleCoord += float2(0.0f, 2.0f) * InverseSceneSize;
    }

    return min(2.0f * index + 2.0f * edge, 2.0f * MaxSearchSteps);
}

float4 mad(float4 m, float4 a, float4 b)
{
    return m * a + b;
}

float2 Area(float2 distance, float e1, float e2) 
{
     // * By dividing by areaSize - 1.0 below we are implicitely offsetting to
     //   always fall inside of a pixel
     // * Rounding prevents bilinear access precision problems
    float areaSize = MAX_DISTANCE * 5;
    float2 pixcoord = MAX_DISTANCE * round(4.0f * float2(e1, e2)) + distance;
    float2 texcoord = pixcoord / (areaSize - 1.0);

    return Texture1.SampleLevel(LinearSampler, texcoord, 0).rg;
}

float4 PS_BlendWeight(PS_In_Quad input) : SV_TARGET0
{
    float4 weights = 0.0;
    float2 edges = Texture0.SampleLevel(PointSampler, input.vTexCoord, 0).xy;

    [branch]
    if (edges.y) // Edge at north
    {
        // Search distances to the left and to the right
        float2 dist = float2(SearchXLeft(input.vTexCoord), SearchXRight(input.vTexCoord));

        // Now fetch the crossing edges. Instead of sampling between edgels, we
        // sample at -0.25, to be able to discern what value has each edgel
        float4 vEdgeCoords = mad(float4(dist.x, -0.25f, dist.y + 1.0f, -0.25f),
                                 InverseSceneSize.xyxy, input.vTexCoord.xyxy);

        float e1 = Texture0.SampleLevel(LinearSampler, vEdgeCoords.xy, 0).r;
        float e2 = Texture0.SampleLevel(LinearSampler, vEdgeCoords.zw, 0).r;

        // Ok, we know how this pattern looks like, now it is time for getting the actual area
        weights.rg = Area(abs(dist), e1, e2);
    }

    [branch]
    if (edges.x) // Edge at west
    {
        // Search distances to the top and to the bottom:
        float2 dist = float2(SearchYUp(input.vTexCoord), SearchYDown(input.vTexCoord));
        
        // Now fetch the crossing edges (yet again):
        float4 coords = mad(float4(-0.25f, dist.x, -0.25f, dist.y + 1.0f),
                            InverseSceneSize.xyxy, input.vTexCoord.xyxy);

        float e1 = Texture0.SampleLevel(LinearSampler, coords.xy, 0).g;
        float e2 = Texture0.SampleLevel(LinearSampler, coords.zw, 0).g;

        // Get the area for this direction
        weights.ba = Area(abs(dist), e1, e2);
    }

    return saturate(weights);
}

float4 PS_CopyBackground(PS_In_Quad input) : SV_TARGET0
{
    return Texture0.SampleLevel(PointSampler, input.vTexCoord, 0);
}

float4 PS_NeighborhoodBlend(PS_In_Quad input) : SV_TARGET0
{
    // Fetch the blending weights for current pixel:
    float4 topLeft = Texture1.SampleLevel(PointSampler, input.vTexCoord, 0);
    float bottom = Texture1.SampleLevel(PointSampler, input.vTexCoord, 0, int2(0, 1)).g;
    float right = Texture1.SampleLevel(PointSampler, input.vTexCoord, 0, int2(1, 0)).a;
    float4 a = float4(topLeft.r, bottom, topLeft.b, right);

    // Up to 4 lines can be crossing a pixel (one in each edge). So, we perform
    // a weighted average, where the weight of each line is 'a' cubed, which
    // favors blending and works well in practice.
    float4 w = a * a * a;

    // There is some blending weight with a value greater than 0.0?
    float sum = dot(w, 1.0f);

    [branch]
    if (sum < EPSILON)
    {
        discard;
    }

    float4 color = 0.0;

    // Add the contributions of the possible 4 lines that can cross this pixel
    float4 coords = mad(float4( 0.0f, -a.r, 0.0f,  a.g), InverseSceneSize.yyyy, input.vTexCoord.xyxy);
    color = mad(Texture0.SampleLevel(LinearSampler, coords.xy, 0), w.r, color);
    color = mad(Texture0.SampleLevel(LinearSampler, coords.zw, 0), w.g, color);

    coords = mad(float4(-a.b,  0.0f, a.a, 0.0f), InverseSceneSize.xxxx, input.vTexCoord.xyxy);
    color = mad(Texture0.SampleLevel(LinearSampler, coords.xy, 0), w.b, color);
    color = mad(Texture0.SampleLevel(LinearSampler, coords.zw, 0), w.a, color);

    // Normalize the resulting color and we are finished!
    return color / sum;
}
