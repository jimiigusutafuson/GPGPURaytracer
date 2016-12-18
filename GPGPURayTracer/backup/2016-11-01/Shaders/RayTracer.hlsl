//--------------------------------------------------------------------------------------
// RayTracer.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

#include "Structs.hlsl"
#include "CBuffers.hlsl"

//predefined from CPU (see mMacros in the ComputeShader header file):
//BLOCK_SIZE_X
//BLOCK_SIZE_Y
//BLOCK_SIZE_Z
//N (block size x times grid size x)

//definitions
#define defaultColor float4(0.9,0.3,0.4,0);
#define FLT_MAX 3.402823466e+38F;

//samplers
SamplerState samLinear	: register(s0);

//SRV
//StructuredBuffer<Vertex>			g_vBuffer			: register(t0); //vertex buffer
Texture2DArray						g_tex				: register(t0); //kd textures
Texture2DArray						g_texN				: register(t1); //normals
StructuredBuffer<Material>			g_materials			: register(t2); //materials
StructuredBuffer<TriangleInfo>		g_triangleInfo		: register(t3); //indices for each triangle of the material and model used
//UAV
RWTexture2D<float4>					g_output			: register(u0); //final texture
RWStructuredBuffer<Vertex>			g_vBuffer			: register(u1); //transformedvertex buffer
RWStructuredBuffer<Ray>				g_rays				: register(u2); //rays generated in the primary stage
RWStructuredBuffer<float4>			g_accumulation		: register(u3); //accumulating colors
RWStructuredBuffer<Intersection>	g_intersections		: register(u4); //all intersections
RWStructuredBuffer<float4x4>		g_worlds			: register(u5); //all world matrices (size and indices in this buffer are equivalent to the amount of models)

//dependent shaders
#include "Intersections.hlsl"

//compute shaders
#include "CSPrimaryRays.hlsl"
#include "CSIntersections.hlsl"
#include "CSColorComputation.hlsl"
#include "CSWorldInteraction.hlsl"