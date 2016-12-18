//--------------------------------------------------------------------------------------
// CSColorComputation.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

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
	const unsigned int index = threadID.y * N + threadID.x;				//ID of the pixel for all buffers
	const int triangleID = g_intersections[index].triangleID;			//get the triangleID for this pixel's intersection;
	const int currentMaterial = g_triangleInfo[triangleID].material;	//get the index of the current material used based on the triangle ID
	float3 faceNormal;
	float3 finalNormal;
	float4 finalColor;
	const float2 texCoords = getTexCoords(triangleID, g_intersections[index]);
	int offset = triangleID * 3;
	if (triangleID < g_envMappingFlag)
	{
		g_output[threadID.xy] = float4(1, 0, 0, 1);
		return;
	}
	else if (triangleID >= 0) //hit something
	{
		//2 vectors spanning the triangle
		float3 edge1 = g_vBuffer[offset + 1].pos - g_vBuffer[offset].pos;
		float3 edge2 = g_vBuffer[offset + 2].pos - g_vBuffer[offset].pos;
		//----------------------------------------------------
		//                 normal generation
		//----------------------------------------------------
		if (g_isPhongShadingOn) //perform phong shading interpolation of normals
		{
			faceNormal = normalize(baryCentric(g_vBuffer[offset].norm, g_vBuffer[offset + 1].norm, g_vBuffer[offset + 2].norm,
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

			float2 uv1 = g_vBuffer[offset + 1].texCoord - g_vBuffer[offset].texCoord;
			float2 uv2 = g_vBuffer[offset + 2].texCoord - g_vBuffer[offset].texCoord;
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

		float4 ambient = float4(g_materials[currentMaterial].ka,1.0f); // float4(0.2f, 0.2f, 0.2f, 1.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
		
		if (g_isShadowOn)
		{
			int tr;
			Ray rayTowardsLight = g_rays[index];
			rayTowardsLight.origin = hitPoint;
			rayTowardsLight.triangleID = triangleID;

			//POINT LIGHTS
			for (int i = 0; i < g_numPointLights; i++) //for every light
			{
				tr = -1;
				float3 lightDir = g_pointLights[i].pos - hitPoint; //light direction
				const float lightDistance = length(lightDir); //store the distance
				lightDir = normalize(lightDir); //normalize light direction
				float lightDistanceFactor = saturate(1.0f - (lightDistance / g_pointLights[i].reach)); //closer lights have better effect
																									   // Cast shadows
				rayTowardsLight.direction = lightDir;
				rayTowardsLight.maxT = lightDistance;
				if (g_isBVHUsed) //use the BVH ray intersection
				{

				}
				else //use normal ray intersection
				{
					tr = simpleVBufferIntersection(rayTowardsLight).triangleID;
				}
				if (tr < 0) //if no triangles were hit towards the light source
				{
					float4 lightColor = lightDistanceFactor * float4(g_pointLights[i].color, 1.0f);
					diffuse += max(0.0f, dot(lightDir, finalNormal)) * lightColor;
					specular += pow(max(0.f, dot(lightDir, reflectedDir)), g_materials[currentMaterial].ns) * lightColor;
				}
			}
		}
		else
		{
			for (int i = 0; i < g_numPointLights; i++) //for every light
			{
				float3 lightDir = g_pointLights[i].pos - hitPoint; //light direction
				const float lightDistance = length(lightDir); //store the distance
				lightDir = normalize(lightDir); //normalize light direction
				float lightDistanceFactor = 1.0f - (lightDistance / g_pointLights[i].reach); //closer lights have better effect
				float4 lightColor = lightDistanceFactor * float4(g_pointLights[i].color, 1.0f);
				diffuse += max(0.0f, dot(lightDir, finalNormal)) * lightColor;
				specular += pow(max(0.f, dot(lightDir, reflectedDir)), g_materials[currentMaterial].ns) * lightColor;
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

float shadow(Ray ray, float rayLength)
{
	int index = 0;
	int i;
	float a, euro, f, u, v;
	float3 e1, e2, q, s, r;
	//float3 normal;
	float t;
	for (i = 0; i < noVertices; i += 3)
	{
		Triangle tri;
		tri.v0 = index++;
		tri.v1 = index++;
		tri.v2 = index++;


		e1 = g_vBuffer[tri.v1].pos - g_vBuffer[tri.v0].pos;
		e2 = g_vBuffer[tri.v2].pos - g_vBuffer[tri.v0].pos;
		//normal = normalize(cross(e1, e2));
		q = cross(e2, ray.direction.xyz);
		a = dot(e1, q);
		euro = 0.00001;

		if (a > -euro && a < euro)
			continue;

		f = 1 / a;
		s = ray.origin.xyz - g_vBuffer[tri.v0].pos;

		u = f*dot(s, q);

		if (u < 0.0)
			continue;

		r = cross(e1, s);
		v = f*dot(ray.direction.xyz, r);
		if (v < 0.0 || u + v > 1.0)
			continue;

		t = f*dot(e2, r);

		if (t > 0.1 && t < rayLength) //raylength should prevent the ray from continue after it passes the lightsource
		{
			return 0.2 * (1 - t / rayLength);
		}
	}
	return 1.0;
}

Vertex interpolate(float3 hitPos, Triangle tri)
{
	float3 p1 = g_vBuffer[tri.v0].pos;
	float3 p2 = g_vBuffer[tri.v1].pos;
	float3 p3 = g_vBuffer[tri.v2].pos;
	//vectors from triangle points to hitpos
	float3 p1Hit = p1 - hitPos;
	float3 p2Hit = p2 - hitPos;
	float3 p3Hit = p3 - hitPos;

	float area = length(cross(p1 - p2, p1 - p3));	//area of the parallelogram the vectors form for the entire triangle
	float a1 = length(cross(p2Hit, p3Hit)) / area;	//area of the parallelogram on the opposite side of p1
	float a2 = length(cross(p3Hit, p1Hit)) / area;	//area of the parallelogram on the opposite side of p2
	float a3 = length(cross(p1Hit, p2Hit)) / area;	//area of the parallelogram on the opposite side of p3

	Vertex vOut;
	vOut.pos = hitPos;
	vOut.norm = normalize(g_vBuffer[tri.v0].norm * a1 + g_vBuffer[tri.v1].norm * a2 + g_vBuffer[tri.v2].norm * a3);
	vOut.texCoord = g_vBuffer[tri.v0].texCoord * a1 + g_vBuffer[tri.v1].texCoord * a2 + g_vBuffer[tri.v2].texCoord * a3;

	return vOut;
}

//void renderMesh(inout HitData hitData, Ray ray, int start, int end)
//{
//	//declarations
//	float3 e1, e2, d, q, s, o, r;
//	float a, f, u, v;
//	float euro = 0.00001;				//a constant used to avoid determinants close to 0.
//	float newT;							//the new t value
//	d = ray.direction.xyz;				//we use d for the ray direction
//	o = ray.origin.xyz;					//and o for the ray origin
//	Triangle hittri, tri;				//triangle indices
//	int i;
//	int index = start;
//	bool hit = false;
//	for (i = 0; i < end; i += 3)
//	{
//		tri.v0 = index++;
//		tri.v1 = index++;
//		tri.v2 = index++;
//
//		e1 = g_vBuffer[tri.v1].pos - g_vBuffer[tri.v0].pos;
//		e2 = g_vBuffer[tri.v2].pos - g_vBuffer[tri.v0].pos;
//		q = cross(d, e2);					//cross the direction vector and one of the spanning vectors for a vector q
//		a = dot(e1, q);						//create the determinant of the matrix M.  M = [-d, p1 - p0, p2 - p0] 
//		if (a > -euro && a < euro)			//if the determinant is too close to 0, abort
//			continue;
//		//generate u, the "x-coordinate" of the intersection point in the triangle
//		f = 1.0 / a;
//		s = o - g_vBuffer[tri.v0].pos;
//		u = f*dot(s, q);
//		if (u < 0.0)						//compare u to the edge of a triangle
//			continue;
//		r = cross(s, e1);
//		v = f*dot(d, r);					//generate v, the "y-coordinate" of the intersection point in the triangle
//											//if (v < 0.0 || u + v > 1.0)			//compare v and u to the edges of the triangle
//											//	return;
//		newT = f*dot(e2, r);				//says q instead of r in the book, but that is incorrect
//											//the following if statement was combined with the previous to reduce branching
//		if (v < 0.0 || u + v > 1.0 ||		//compare v and u to the edges of the triangle
//			newT > hitData.t || newT < 0)	//if hitData.t is negative, then the intersection is behind us
//			continue;
//		hitData.t = newT;
//		hittri = tri;
//		hit = true;
//	}
//	if (!hit)
//		return;
//
//	float3 lightInt = 0.0;
//	Vertex vInter = interpolate((ray.origin + ray.direction*hitData.t).xyz, hittri);
//	for (i = 0; i < g_numPointLights; i++)
//	{
//		Ray ray;
//		ray.origin = vInter.pos;
//		ray.direction = normalize(g_pointLights[i].pos.xyz - vInter.pos);
//		lightInt += shadow(ray, length(g_pointLights[i].pos.xyz - vInter.pos)) * saturate(g_pointLights[i].color.xyz * dot(normalize(g_pointLights[i].pos.xyz - vInter.pos), vInter.norm)) * (g_pointLights[i].reach / dot(g_pointLights[i].pos.xyz - vInter.pos.xyz, g_pointLights[i].pos.xyz - vInter.pos.xyz));
//		//http://brooknovak.wordpress.com/2008/11/13/hlsl-per-pixel-point-light-using-phong-blinn-lighting-model/
//		//http://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/
//	}
//	hitData.color = g_tex.SampleLevel(samLinear, vInter.texCoord, 0) * float4(lightInt, 0);
//}



//[numthreads(BLOCK_SIZE_X, BLOCK_SIZE_Y, 1)]
//void csComputeColor(uint3 threadID : SV_DispatchThreadID)
//{
//	const unsigned int index = threadID.y * N + threadID.x;				//ID of the pixel for all buffers
//	const int triangleID = g_intersections[index].triangleID;			//get the triangleID for this pixel's intersection;
//	const int currentMaterial = g_materialIndices[triangleID];			//get the index of the current material used based on the triangle ID
//	float3 faceNormal;
//	float3 finalNormal;
//	float4 finalColor;
//	const float2 texCoords = getTexCoords(triangleID, g_intersections[index]);
//	int offset = triangleID * 3;
//	if (triangleID < g_envMappingFlag)
//	{
//		g_output[threadID.xy] = float4(1, 0, 0, 1);
//		return;
//	}
//	else if (triangleID >= 0) //hit something
//	{
//		//2 vectors spanning the triangle
//		float3 edge1 = g_vBuffer[offset + 1].pos - g_vBuffer[offset].pos;
//		float3 edge2 = g_vBuffer[offset + 2].pos - g_vBuffer[offset].pos;
//		//----------------------------------------------------
//		//                 normal generation
//		//----------------------------------------------------
//		faceNormal = normalize(baryCentric(g_vBuffer[offset].norm, g_vBuffer[offset + 1].norm, g_vBuffer[offset + 2].norm,
//			float2(g_intersections[index].u, g_intersections[index].v)));
//		finalNormal = faceNormal;
//
//		if (g_isNormalMapspingOn && g_materials[currentMaterial].map_normal >= 0) //using normal mapping
//		{
//			float3 mapNormal = g_texN.SampleLevel(samLinear, float3(texCoords, g_materials[currentMaterial].map_normal), 0.0f).xyz;
//			if (length(mapNormal) < 0.1) //probably the texture is not correct
//			{
//				mapNormal = float3(0.5f, 0.5f, 1.0f); //set the normal to point straight up
//			}
//			mapNormal = 2 * mapNormal - 1; //convert from pixel format to vector format (centering)
//
//			float2 uv1 = g_vBuffer[offset + 1].texCoord - g_vBuffer[offset].texCoord;
//			float2 uv2 = g_vBuffer[offset + 2].texCoord - g_vBuffer[offset].texCoord;
//			const float cp = uv1.y*uv2.x - uv1.x*uv2.y;
//			if (cp != 0.0f)
//			{
//				//we need to get the tangent and bitangent to project the normal to the face
//				float multiplier = 1.0f / cp;
//				float3 tangent, bitangent;
//				tangent = (edge2*uv1.y - edge1*uv2.y)*multiplier;
//				bitangent = (edge2*uv1.x - edge1*uv2.x)*multiplier;
//				tangent = normalize(tangent - faceNormal*dot(tangent, faceNormal));
//				bitangent = bitangent - faceNormal*dot(bitangent, faceNormal);
//				bitangent = normalize(bitangent - tangent*dot(bitangent, tangent));
//				finalNormal = normalize(mapNormal.z*faceNormal + mapNormal.x*tangent - mapNormal.y*bitangent);
//			}
//		}
//		//----------------------------------------------------
//		//                 light calculations
//		//----------------------------------------------------
//		float3 hitPoint = g_rays[index].origin + g_intersections[index].t * g_rays[index].direction;
//
//		// Ray reflection
//		const float3 reflectedDir = normalize(reflect(g_rays[index].direction, finalNormal));
//
//		float4 ambient = float4(g_materials[currentMaterial].ka, 1.0f); // float4(0.2f, 0.2f, 0.2f, 1.0f);
//		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
//		float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
//
//		int tr;
//		Ray rayTowardsLight = g_rays[index];
//		rayTowardsLight.origin = hitPoint;
//		rayTowardsLight.triangleID = triangleID;
//
//		for (int i = 0; i < g_numPointLights; i++) //for every light
//		{
//			tr = -1;
//			float3 lightDir = g_pointLights[i].pos - hitPoint; //light direction
//			const float lightDistance = length(lightDir); //store the distance
//			lightDir = normalize(lightDir); //normalize light direction
//			float lightDistanceFactor = saturate(1.0f - (lightDistance / g_pointLights[i].reach)); //closer lights have better effect
//																									// Cast shadows
//			rayTowardsLight.direction = lightDir;
//			rayTowardsLight.maxT = lightDistance;
//
//			tr = simpleVBufferIntersection(rayTowardsLight).triangleID;
//			if (tr < 0) //if no triangles were hit towards the light source
//			{
//				float4 lightColor = lightDistanceFactor * float4(g_pointLights[i].color, 1.0f);
//				diffuse += max(0.0f, dot(lightDir, finalNormal)) * lightColor;
//				specular += pow(max(0.f, dot(lightDir, reflectedDir)), g_materials[currentMaterial].ns) * lightColor;
//			}
//		}
//
//		//multiply by the specified kd and ks values from the material
//		diffuse *= float4(g_materials[currentMaterial].kd, 0.0f);
//		specular *= float4(g_materials[currentMaterial].ks, 0.0f);
//
//		finalColor = (ambient + diffuse) * g_tex.SampleLevel(samLinear, float3(texCoords, g_materials[currentMaterial].map_kd), 0.0f);
//		finalColor += specular;
//		finalColor *= float4(g_rays[index].reflectiveFactor, 1.0f);
//
//		//Bounce the ray
//		g_rays[index].reflectiveFactor *= 0.3f;
//		g_rays[index].origin = hitPoint;
//		g_rays[index].direction = reflectedDir;
//		g_rays[index].triangleID = triangleID;
//
//		g_accumulation[index] += finalColor;
//		g_output[threadID.xy] = g_accumulation[index];
//	}
//}