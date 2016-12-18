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
StructuredBuffer<Vertex>			g_vBuffer			: register(t0); //vertex buffer
Texture2DArray						g_tex				: register(t1); //kd textures
Texture2DArray						g_texN				: register(t2); //normals
StructuredBuffer<Material>			g_materials			: register(t3); //materials
StructuredBuffer<TriangleInfo>		g_triangleInfo		: register(t4); //indices for each triangle of the material and model used
//UAV
RWTexture2D<float4>					g_output			: register(u0); //final texture
RWStructuredBuffer<Ray>				g_rays				: register(u1); //rays generated in the primary stage
RWStructuredBuffer<float4>			g_accumulation		: register(u2); //accumulating colors
RWStructuredBuffer<Intersection>	g_intersections		: register(u3); //all intersections
RWStructuredBuffer<PointLight>		g_pointLights		: register(u4); //all point lights
RWStructuredBuffer<float4x4>		g_worlds			: register(u5); //all world matrices (size and indices in this buffer are equivalent to the amount of models)
RWStructuredBuffer<Vertex>			g_vBuffer2			: register(u6); //transformedvertex buffer

//dependent shaders
#include "Intersections.hlsl"

//compute shaders
#include "CSPrimaryRays.hlsl"
#include "CSIntersections.hlsl"
#include "CSColorComputation.hlsl"
#include "CSWorldInteraction.hlsl"