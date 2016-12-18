//--------------------------------------------------------------------------------------
// CSColorComputation.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

//predefinitions to avoid unnecessary calculations
#define PI 3.14159265f
#define COSTHETA 0.995184727f
#define COSPHI 0.707106781f

//obtains the texture coordinates of the selected triangle, and returns an interpolated texture coordinate
float2 getTexCoords(int triangleID, Intersection intersection)
{
	int offset = triangleID * 3;
	float2 v0, v1, v2;

	v0.xy = g_vBuffer[offset].texCoord;
	v1.xy = g_vBuffer[offset + 1].texCoord;
	v2.xy = g_vBuffer[offset + 2].texCoord;

	return intersection.u* v1.xy + intersection.v * v2.xy + (1.0f - (intersection.u + intersection.v))*v0;
}
[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1)]
void csComputeColor(uint3 threadID : SV_DispatchThreadID)
{
	Vertex v1, v2, v3;
	const unsigned int index = threadID.y * N + threadID.x;				//ID of the pixel for all buffers
	const int triangleID = g_intersections[index].triangleID;			//get the triangleID for this pixel's intersection;
	const int currentMaterial = g_triangleInfo[triangleID].material;	//get the index of the current material used based on the triangle ID
	float3 faceNormal;
	float3 finalNormal;
	float4 finalColor;
	const float2 texCoords = getTexCoords(triangleID, g_intersections[index]);
	int offset = triangleID * 3; //get the vertices connected to this triangleID (quads, etc, are not supported)
	v1 = g_vBuffer[offset];
	v2 = g_vBuffer[offset + 1];
	v3 = g_vBuffer[offset + 2];
	g_rays[index].triangleID = -2; //this will disable the ray next pass to reduce collision time (if the ray hit something this round, it will change)
	if (triangleID < g_envMappingFlag)
	{
		g_output[threadID.xy] = float4(1, 0, 0, 1);
		return;
	}
	else if (triangleID >= 0) //hit something
	{
		//2 vectors spanning the triangle
		float3 edge1 = v2.pos - v1.pos;
		float3 edge2 = v3.pos - v1.pos;
		//----------------------------------------------------
		//                 normal generation
		//----------------------------------------------------
		if (g_isPhongShadingOn) //perform phong shading interpolation of normals
		{
			faceNormal = normalize(baryCentric(v1.norm, v2.norm, v3.norm,
				float2(g_intersections[index].u, g_intersections[index].v)));
		}
		else //generate vector-based Normal
		{
			faceNormal = normalize(cross(edge1, edge2));
			if (dot(faceNormal, g_rays[index].direction) > 0.0f) //make sure the generated normal is pointing outwards, not inwards
			{
				faceNormal = -faceNormal;
			}
		}
		finalNormal = faceNormal;

		if (g_isNormalMapspingOn && g_materials[currentMaterial].map_normal >= 0) //using normal mapping
		{
			float3 mapNormal = g_texN.SampleLevel(samLinear, float3(texCoords, g_materials[currentMaterial].map_normal), 0.0f).xyz;
			if (length(mapNormal) < 0.1) //probably the texture is not correct
			{
				mapNormal = float3(0.5f, 0.5f, 1.0f); //set the normal to point straight up
			}
			mapNormal = 2 * mapNormal - 1; //convert from pixel format to vector format (centering)

			float2 uv1 = v2.texCoord - v1.texCoord;
			float2 uv2 = v3.texCoord - v1.texCoord;
			const float cp = uv1.y*uv2.x - uv1.x*uv2.y;
			if (cp != 0.0f)
			{
				//we need to get the tangent and bitangent to project the normal to the face
				float multiplier = 1.0f / cp;
				float3 tangent, bitangent;
				tangent = (edge2*uv1.y - edge1*uv2.y)*multiplier;
				bitangent = (edge2*uv1.x - edge1*uv2.x)*multiplier;
				tangent = normalize(tangent - faceNormal*dot(tangent, faceNormal));
				bitangent = bitangent - faceNormal*dot(bitangent, faceNormal);
				bitangent = normalize(bitangent - tangent*dot(bitangent, tangent));
				finalNormal = normalize(mapNormal.z*faceNormal + mapNormal.x*tangent - mapNormal.y*bitangent);
			}
		}
		//----------------------------------------------------
		//                 light calculations
		//----------------------------------------------------
		float3 hitPoint = g_rays[index].origin + g_intersections[index].t * g_rays[index].direction;

		// Ray reflection
		const float3 reflectedDir = normalize(reflect(g_rays[index].direction, finalNormal));

		float4 ambient = float4(g_materials[currentMaterial].ka,1.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
		
		if (g_isShadowOn)
		{
			int tr;
			Ray rayTowardsLight = g_rays[index];
			rayTowardsLight.origin = hitPoint;
			rayTowardsLight.triangleID = triangleID;
			//the code is split up into 4 different big branches. It looks ugly, but it is better performance-wise to branch early, than inside the loop.
			if (g_lightType == 0) //POINT LIGHTS
			{
				for (int i = 0; i < g_numLights; i++) //for every light
				{
					tr = -1;
					float3 lightDir = g_lightPos[i] - hitPoint; //light direction
					const float lightDistance = length(lightDir); //store the distance
					lightDir = normalize(lightDir); //normalize light direction
					float lightDistanceFactor = saturate(1.0f - (lightDistance / g_lightDir[i].w)); //closer lights have better effect
																										   // Cast shadows
					rayTowardsLight.direction = lightDir;
					rayTowardsLight.maxT = lightDistance;
					if (g_isBVHUsed) //use the BVH ray intersection
					{
						//put BVH call here
					}
					else //use normal ray intersection
					{
						tr = simpleVBufferIntersection(rayTowardsLight).triangleID;
					}
					if (tr < 0) //if no triangles were hit towards the light source
					{
						float4 lightColor = lightDistanceFactor * g_lightColor[i];
						diffuse += max(0.0f, dot(lightDir, finalNormal)) * lightColor;
						specular += pow(max(0.f, dot(lightDir, reflectedDir)), g_materials[currentMaterial].ns) * lightColor;
					}
				}
			}
			else if (g_lightType == 1) //SPOT LIGHTS
			{
				for (int i = 0; i < g_numLights; i++) //for every light
				{
					tr = -1;
					const float cosTheta = COSTHETA; // cos(PI / 32);
					const float cosPhi = COSPHI; // cos(PI / 4);  //replace with definition to skip unnecessary computation
					float3 lightDir = g_lightPos[i].xyz - hitPoint; //light direction for this pixel (not to be confused with the light's focused direction)
					const float lightLength = length(lightDir);
					lightDir = normalize(lightDir); //normalize light direction
					float lightReach = g_lightDir[i].w;
					rayTowardsLight.direction = lightDir;
					rayTowardsLight.maxT = lightLength;
					if (g_isBVHUsed) //use the BVH ray intersection
					{

					}
					else //use normal ray intersection
					{
						tr = simpleVBufferIntersection(rayTowardsLight).triangleID;
					}
					if (tr < 0 && lightLength < lightReach)
					{
						float attenuation = 0.0;
						float cosAlpha = max(dot(-lightDir, g_lightDir[i].xyz), 0.0);
						if (cosAlpha > cosTheta)
							attenuation = 1.0;
						else if (cosAlpha > cosPhi)
							attenuation = pow((cosAlpha - cosPhi) / (cosTheta - cosPhi), 20.0);
						float4 lightFade = ((lightReach - lightLength) / lightReach) * g_lightColor[i];
						diffuse += max(dot(lightDir, finalNormal), 0.0) * attenuation * lightFade;
						specular += pow(max(0.f, dot(lightDir, reflectedDir)), g_materials[currentMaterial].ns) * attenuation * lightFade;
					}
				}
			}
		}
		else
		{
			if (g_lightType == 0) //POINT LIGHTS
			{
				for (int i = 0; i < g_numLights; i++) //for every light
				{
					float3 lightDir = g_lightPos[i] - hitPoint; //light direction
					const float lightDistance = length(lightDir); //store the distance
					lightDir = normalize(lightDir); //normalize light direction
					float lightDistanceFactor = saturate(1.0f - (lightDistance / g_lightDir[i].w)); //closer lights have better effect
					float4 lightColor = lightDistanceFactor * g_lightColor[i];
					diffuse += max(0.0f, dot(lightDir, finalNormal)) * lightColor;
					specular += pow(max(0.f, dot(lightDir, reflectedDir)), g_materials[currentMaterial].ns) * lightColor;
				}
			}
			else if (g_lightType == 1) //SPOT LIGHTS
			{
				for (int i = 0; i < g_numLights; i++) //for every light
				{
					const float cosTheta = COSTHETA; // cos(PI / 32);
					const float cosPhi = COSPHI; // cos(PI / 4);
					float3 lightDir = g_lightPos[i].xyz - hitPoint; //light direction for this pixel (not to be confused with the light's focused direction)
					const float lightLength = length(lightDir);
					lightDir = normalize(lightDir); //normalize light direction
					float lightReach = g_lightDir[i].w;
					if (lightLength < lightReach)
					{
						float attenuation = 0.0;
						float cosAlpha = max(dot(-lightDir, g_lightDir[i].xyz), 0.0);
						if (cosAlpha > cosTheta)
							attenuation = 1.0;
						else if (cosAlpha > cosPhi)
							attenuation = pow((cosAlpha - cosPhi) / (cosTheta - cosPhi), 20.0);
						float4 lightFade = ((lightReach - lightLength) / lightReach) * g_lightColor[i];
						diffuse += max(dot(lightDir, finalNormal), 0.0) * attenuation * lightFade;
						specular += pow(max(0.f, dot(lightDir, reflectedDir)), g_materials[currentMaterial].ns) * attenuation * lightFade;
					}
				}
			}
		}
		
		//multiply by the specified kd and ks values from the material
		diffuse *= float4(g_materials[currentMaterial].kd, 0.0f);
		specular *= float4(g_materials[currentMaterial].ks, 0.0f);
		
		finalColor  = (ambient + diffuse) * g_tex.SampleLevel(samLinear, float3(texCoords, g_materials[currentMaterial].map_kd), 0.0f);
		finalColor += specular;
		finalColor *= float4(g_rays[index].reflectiveFactor, 1.0f);

		//Bounce the ray
		g_rays[index].reflectiveFactor *= 0.3f;
		g_rays[index].origin = hitPoint;
		g_rays[index].direction = reflectedDir;
		g_rays[index].triangleID = triangleID;

		g_accumulation[index] += finalColor;
		g_output[threadID.xy] = g_accumulation[index];
	}
}