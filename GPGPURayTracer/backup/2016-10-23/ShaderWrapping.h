#pragma once
#include <directxmath.h>
//all the shader wrapping data (structs, cbuffers, etc)

struct CBOnce
{
	DirectX::XMFLOAT4	resolution;
};

struct CBCamera
{
	DirectX::XMFLOAT4	pos;
	DirectX::XMFLOAT4	dir;
	DirectX::XMFLOAT4    up;
};

struct CBVertexInfo
{
	int			noVertices;
	int			noTriangles;
	DirectX::XMFLOAT2	padding;
};

struct CBPointLight
{
	DirectX::XMFLOAT4 positions[10];
	DirectX::XMFLOAT4 colorReach[10];	//color(xyz) AND reach(w)
	int	noLights;				//number of lights
	DirectX::XMFLOAT3 padding;
};
struct CBSettings
{
	int	g_isShadowOn;					//1 if shadows are on
	int	g_isPhongShadingOn;				//1 if phong shading is on
	int	g_isNormalMapspingOn;			//1 if normal mapping is on
	int	g_isGlossMappingOn;				//1 if gloss mapping is on
	int	g_isBVHUsed;					//1 if BVH is used
	int	g_envMappingFlag;				//1 if environment mapping is on
	unsigned int g_numBounces;			//the amount of bounces
	int g_numPointLights;				//the number of point lights
};