//--------------------------------------------------------------------------------------
// CSPrimaryRays.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1)]
void csGeneratePrimaryRays( uint3 threadID : SV_DispatchThreadID )
{
	float3 right = cross(camUp.xyz, camDir.xyz);

	//generate pixel positions
	/*float inverse = 1.0f / (float(N));
	float y = -float(2.f * threadID.y + 1.f - N) * inverse;
	float x = float(2.f * threadID.x + 1.f - N) * inverse;
	float z = 2.0f;
	float3 pixelPosition = float3(x, y, z); //multiply this with a world pos to orient the ray correctly for objects with world matrices*/
	float inv = 1.0f / float(N);
	float x = (threadID.x * inv) - 0.5;
	float y = (threadID.y * inv) - 0.5;
	float3 pixelPosition = x * right - y * camUp.xyz + camPos.xyz + camDir.xyz;	//calculate the pixels pos in worldspace
																				// for the screenpixel this thread is aimed at

	HitData hitData;
	hitData.t = FLT_MAX;
	hitData.color = defaultColor;
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