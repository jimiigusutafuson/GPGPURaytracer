//--------------------------------------------------------------------------------------
// RayTracer.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

#include "Structs.hlsl"
#include "CBuffers.hlsl"
#include "Intersections.hlsl"

//definitions
#define defaultColor float4(0.9,0.3,0.4,0);
#define FLT_MAX 3.402823466e+38F;

//samplers
SamplerState samLinear	: register(s0);
SamplerState testSampler : IMMUTABLE
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

//SRV
Texture2D					tex		: register(t0);
StructuredBuffer<Vertex>	vBuffer	: register(t1);
StructuredBuffer<int>		iBuffer	: register(t2); //currently not used

//UAV
RWTexture2D<float4>			output	: register(u0);

//read and write
//RWStructuredBuffer<Vertex> vBuffer : register(u1); //vertices
//RWStructuredBuffer<Sphere> vSpheres : register(u2); //spheres
//read only

//compute shaders
#include "CSPrimaryRays.hlsl"
#include "CSIntersections.hlsl"
#include "CSColorComputation.hlsl"