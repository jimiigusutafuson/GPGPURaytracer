//--------------------------------------------------------------------------------------
// CSIntersections.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1)]
void csComputeIntersections(uint3 threadID : SV_DispatchThreadID)
{
	unsigned int index = threadID.y * N + threadID.x;

	if (g_rays[index].triangleID > (-2))
	{
		g_intersections[index] = simpleVBufferIntersection(g_rays[index]);
	}
	//else
	//{
	//	g_intersections[index].triangleID = -2;
	//}
}