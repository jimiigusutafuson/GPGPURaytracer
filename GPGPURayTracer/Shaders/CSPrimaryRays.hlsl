//--------------------------------------------------------------------------------------
// CSPrimaryRays.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1)]
void csGeneratePrimaryRays( uint3 threadID : SV_DispatchThreadID )
{
	float3 right = cross(camUp.xyz, camDir.xyz);

	float inv = 1.0f / float(N);
	float x = (threadID.x * inv) - 0.5;
	float y = (threadID.y * inv) - 0.5;
	float3 pixelPosition = x * right - y * camUp.xyz + camPos.xyz + camDir.xyz;	//calculate the pixels pos in worldspace

	Ray ray;
	ray.origin				= camPos.xyz;
	ray.direction			= normalize(pixelPosition - ray.origin.xyz);
	ray.maxT				= FLT_MAX;
	ray.minT				= 0;
	ray.reflectiveFactor	= float3(1.f, 1.f, 1.f);
	ray.triangleID			= -1;

	unsigned int index = threadID.y * N + threadID.x;
	
	g_rays[index]					= ray;	//Copy ray to global UAV
	g_accumulation[index]			= 0.0f;	//Initialize accumulation buffer
	g_output[threadID.xy]			= float4(ray.direction.x, ray.direction.y, ray.direction.z,1);	//testing
}