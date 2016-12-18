//--------------------------------------------------------------------------------------
// Structs.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

struct Ray
{
	float3	origin;
	float3	direction;
};

struct Sphere
{
	float3	pos;
	float4	color;
	float	radius;
};

struct HitData
{
	float4	color;
	float	t;
};

struct Vertex
{
	float3 pos;
	float3 norm;
	float2 texCoord;
};

struct Triangle
{
	int v0;
	int v1;
	int v2;
};
struct Triangle2
{
	float3 v0;
	float3 v1;
	float3 v2;
};