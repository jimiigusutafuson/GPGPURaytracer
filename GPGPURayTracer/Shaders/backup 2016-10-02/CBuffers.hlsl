//--------------------------------------------------------------------------------------
// CBuffers.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

cbuffer cbOnce		: register(b0)
{
	float4 resolution;
};

cbuffer cbCamera	: register(b1)
{
	float4 camPos;		//camera position
	float4 camDir;		//camera Direction
	float4 camUp;		//camera up direction
};

cbuffer cbVertexInfo	: register(b2)
{
	int noVertices;
	float3 padding;
};

cbuffer cbLightSources	: register(b3)
{
	float4 lightPos[10];			//positions
	float4 lightColor_Reach[10];	//color and reach
	int	noLights;					//number of light
	float3 lightPadding;
};