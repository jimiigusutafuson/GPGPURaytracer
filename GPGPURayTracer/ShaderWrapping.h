#pragma once
#include <directxmath.h>
//all the shader wrapping data (structs, cbuffers, etc)

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
struct CBLights								//416 bytes
{
	DirectX::XMFLOAT4	g_lightPos[10];		//position + unused (to prevent weird array packing)
	DirectX::XMFLOAT4	g_lightDir[10];		//direction + reach
	DirectX::XMFLOAT4	g_lightColor[10];	//color + alpha
	int					g_lightType;		//0 pointlight, 1 spotlight
	int					g_numLights;		//number of lights
	DirectX::XMFLOAT2	g_lightPadding;		//padding
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
	int g_settingsPadding;
};