//--------------------------------------------------------------------------------------
// Structs.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

struct Ray
{
	float3	origin;				//origin of ray
	float3	direction;			//normalized direction vector
	float3	reflectiveFactor;	//the factor of which the ray is reflected
	float	maxT;				//maximum distance traveled
	float	minT;				//minimum distance traveled
	int		triangleID;			//ID of last triangle hit
};

struct Vertex
{
	float3 pos;		//vertex position
	float3 norm;	//vertex normal
	float2 texCoord;//vertex texture coordinate
};

struct Triangle
{
	float3 v0;		//position of vertex 0
	float3 v1;		//position of vertex 1
	float3 v2;		//position of vertex 2
};

struct Intersection
{
	int triangleID;	//the index of the triangle hit
	float u, v, t;	//texture coords
};

struct TriangleInfo
{
	int material;	//index of the material used
	int model;		//index of the model used
};

struct PointLight
{
	float3 pos;		//the position of the light
	float3 color;	//the color of the light
	float reach;	//the distance a light reaches
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

//old stuff

//struct IndexTriangle
//{
//	int v0;
//	int v1;
//	int v2;
//};
//struct Sphere
//{
//	float3	pos;
//	float4	color;
//	float	radius;
//};
//
//struct HitData
//{
//	float4	color;
//	float	t;
//};