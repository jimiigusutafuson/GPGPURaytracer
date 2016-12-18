//--------------------------------------------------------------------------------------
// CBuffers.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

cbuffer cbOnce		: register(b0)
{
	float4 resolution;
};

cbuffer cbCamera	: register(b1)		//48 bytes
{
	float4 camPos;						//camera position
	float4 camDir;						//camera Direction
	float4 camUp;						//camera up direction
};

cbuffer cbVertexInfo	: register(b2)	//16 bytes
{
	int noVertices;
	int noTriangles;
	float2 vertexInfoPadding;
};

cbuffer cbSettings : register(b3)		//32  bytes
{
	int	g_isShadowOn;					//1 if shadows are on
	int	g_isPhongShadingOn;				//1 if phong shading is on
	int	g_isNormalMapspingOn;			//1 if normal mapping is on
	int	g_isGlossMappingOn;				//1 if gloss mapping is on
	int	g_isBVHUsed;					//1 if BVH is used
	int	g_envMappingFlag;				//
	uint g_numBounces;					//the amount of bounces
	int g_SettingsPadding;				//padding
};

cbuffer cbLights : register(b4)			//416 bytes
{
	float4 g_lightPos[10];				//position (element 4 is not used) (arrays must align by 16 bytes for each element)
	float4 g_lightDir[10];				//direction (element 4 is the reach)
	float4 g_lightColor[10];			//color (element 4 is alpha)
	int g_lightType;					//0 pointlight, 1 spotlight
	int g_numLights;					//number of lights
	float2 g_lightPadding;				//padding
};