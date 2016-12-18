//--------------------------------------------------------------------------------------
// CSPrimaryRays.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1)]
void csWorldInteraction(uint3 threadID : SV_DispatchThreadID)
{
	const unsigned int index = threadID.y * N + threadID.x;		//ID of the thread
	const int numLoops = noVertices / N;				//amount of loops this thread will perform
	int triangleID;
	int modelID;
	float4x4 mWorld;
	int currIndex;
	Vertex v;

	for (int i = 0; i < numLoops && i < noVertices; i++)
	{
		currIndex = index + i;
		triangleID = (currIndex) / 3;
		modelID = g_triangleInfo[triangleID].model;
		mWorld = g_worlds[modelID];
		v.pos = float4(g_vBuffer2[currIndex].pos, 1.0f);
		v.norm = float4(g_vBuffer2[currIndex].norm, 0.0f);
		v.texCoord = g_vBuffer2[currIndex].texCoord;
		g_vBuffer2[currIndex] = v;
	}
}