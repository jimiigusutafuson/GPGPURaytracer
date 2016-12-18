//--------------------------------------------------------------------------------------
// Structs.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

struct Ray //48 bytes
{
	float3	origin;				//origin of ray
	float3	direction;			//normalized direction vector
	float3	reflectiveFactor;	//the factor of which the ray is reflected
	float	maxT;				//maximum distance traveled
	float	minT;				//minimum distance traveled
	int		triangleID;			//ID of last triangle hit
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

struct Intersection
{
	int triangleID;
	float u, v, t;		//texture coords
	//int visitedNodes;
	//int root;
};

struct TriangleInfo
{
	int material;
	int model;
};

struct PointLight
{
	float3 pos;
	float3 color;
	float reach;
};

struct Material
{
	float ns;		//specular intensity (0-1000)
	float3 ka;		//ambient
	float3 kd;		//diffuse
	float3 ks;		//specular lightning. black = off.
	int	map_kd;		//index of diffuse map
	int map_normal;	//index of normal map
};