cbuffer cbDepthProperties : register(b0)
{
    float4x4 WorldViewProjection;
	float Direction;
	float2 CameraClips;
	float Padding;
}

struct VS_In_Depth
{
	float4 vPositionOS	: POSITION;
};

struct VS_Out_Depth
{
	float4 vPositionCS	: SV_POSITION;
	float z				: Z;
	float2 vDepth		: DEPTH;
};

VS_Out_Depth VS_Depth(VS_In_Depth input)
{/*
    VS_Out_Depth output;

    float4 pos = mul(input.vPositionOS, WorldViewProjection);
	pos.xyz = pos.xyz / pos.www;
	pos.w = 1.0f;

	pos.z = pos.z * Direction.x;

	float fLength = length(pos);
	pos = pos / fLength;

	output.fDepth = pos.z;

	pos.z = pos.z + 1.0f;
	pos.xy = pos.xy / pos.z;

	pos.z = (fLength - CameraClips.x) / (CameraClips.y - CameraClips.x);
	pos.w = 1.0f;

	output.vPositionCS = pos;

    return output;*/

	VS_Out_Depth output;
	//Render with the Dual-Paraboloid distortion
	
	// Transform to homogeneous clip space.
	output.vPositionCS = mul(input.vPositionOS, WorldViewProjection);
	output.vPositionCS = output.vPositionCS / output.vPositionCS.w;
	
	output.vPositionCS.z = output.vPositionCS.z * Direction;
	
	float L = length(output.vPositionCS.xyz);							// determine the distance between (0,0,0) and the vertex
	output.vPositionCS = output.vPositionCS / L;								// divide the vertex position by the distance 
	
	output.z = output.vPositionCS.z;										// remember which hemisphere the vertex is in
	output.vPositionCS.z = output.vPositionCS.z + 1.0f;							// add the reflected vector to find the normal vector

	output.vPositionCS.x = output.vPositionCS.x / output.vPositionCS.z;					// divide x coord by the new z-value
	output.vPositionCS.y = output.vPositionCS.y / output.vPositionCS.z;					// divide y coord by the new z-value

	output.vPositionCS.z = (L - CameraClips.x) / (CameraClips.y - CameraClips.x);	// scale the depth to [0, 1]
	output.vPositionCS.w = 1;											// set w to 1 so there is no w divide
	
	output.vDepth = output.vPositionCS.zw;

	// Done--return the output.
    return output;
}

float4 PS_Depth(VS_Out_Depth input) : SV_TARGET0
{
	clip(input.z);

	//float fDepth = input.vDepth.x / input.vDepth.y;
	float fDepth = input.vPositionCS.z / input.vPositionCS.w;

	return float4(fDepth, fDepth * fDepth, 0.0f, 0.0f);
}