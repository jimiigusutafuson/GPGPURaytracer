//--------------------------------------------------------------------------------------
// CSPrimaryRays.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

Vertex interpolate( float3 hitPos, Triangle tri)
{
	float3 p1 = vBuffer[tri.v0].pos;
	float3 p2 = vBuffer[tri.v1].pos;
	float3 p3 = vBuffer[tri.v2].pos;
	//vectors from triangle points to hitpos
	float3 p1Hit = p1 - hitPos;
	float3 p2Hit = p2 - hitPos;
	float3 p3Hit = p3 - hitPos;

	float area	= length( cross( p1-p2, p1-p3 ) );			//area of the parallelogram the vectors form for the entire triangle
	float a1	= length( cross( p2Hit, p3Hit ) ) / area;	//area of the parallelogram on the opposite side of p1
	float a2	= length( cross( p3Hit, p1Hit ) ) / area;	//area of the parallelogram on the opposite side of p2
	float a3	= length( cross( p1Hit, p2Hit ) ) / area;	//area of the parallelogram on the opposite side of p3

	Vertex vOut;
	vOut.pos		= hitPos;
	vOut.norm		= normalize( vBuffer[tri.v0].norm * a1 + vBuffer[tri.v1].norm * a2 + vBuffer[tri.v2].norm * a3 );
	vOut.texCoord	= vBuffer[tri.v0].texCoord * a1 + vBuffer[tri.v1].texCoord * a2 + vBuffer[tri.v2].texCoord * a3;

	return vOut;
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


		e1 = vBuffer[tri.v1].pos - vBuffer[tri.v0].pos;
		e2 = vBuffer[tri.v2].pos - vBuffer[tri.v0].pos;
		//normal = normalize(cross(e1, e2));
		q = cross(e2, ray.direction.xyz);
		a = dot(e1, q);
		euro = 0.00001;

		if (a > -euro && a < euro)
			continue;

		f = 1 / a;
		s = ray.origin.xyz - vBuffer[tri.v0].pos;

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

void rayVSTriangle(inout HitData hitData, Ray ray)
{
	//---------------------------------old method (working)
	float a, euro, u, v, f;
	float3 e1, q, s, e2, r;
	int index = 0, i;
	Triangle hittri, tri;
	for (i = 0; i < noVertices; i += 3)
	{
		tri.v0 = index++;
		tri.v1 = index++;
		tri.v2 = index++;

		e1 = vBuffer[tri.v1].pos - vBuffer[tri.v0].pos;
		e2 = vBuffer[tri.v2].pos - vBuffer[tri.v0].pos;
		q = cross(e2, ray.direction.xyz);
		a = dot(e1, q);
		euro = 0.00001;

		if (a > -euro && a < euro)
			continue;

		f = 1 / a;
		s = ray.origin.xyz - vBuffer[tri.v0].pos;

		u = f*dot(s, q);

		if (u < 0.0)
			continue;

		r = cross(e1, s);
		v = f*dot(ray.direction.xyz, r);
		if (v < 0.0 || u + v > 1.0)
			continue;

		float newT = f*dot(e2, r);
		if (newT > 0 && newT < hitData.t)
		{
			hitData.t = newT;
			hittri = tri;
		}
	}

	float3 lightInt = 0.0;
	Vertex vInter = interpolate((ray.origin + ray.direction*hitData.t).xyz, hittri);
	for (i = 0; i < noLights; i++)
	{
		Ray ray;
		ray.origin = vInter.pos;
		ray.direction = normalize(lightPos[i].xyz - vInter.pos);
		lightInt += shadow(ray, length(lightPos[i].xyz - vInter.pos)) * saturate(lightColor_Reach[i].xyz * dot(normalize(lightPos[i].xyz - vInter.pos), vInter.norm)) * (lightColor_Reach[i].w / dot(lightPos[i].xyz - vInter.pos.xyz, lightPos[i].xyz - vInter.pos.xyz));
		//http://brooknovak.wordpress.com/2008/11/13/hlsl-per-pixel-point-light-using-phong-blinn-lighting-model/
		//http://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/
	}

	if (vInter.pos.y < 0.001)
		hitData.color = float4(1, 1, 1, 1) * float4(lightInt, 0);
	else
		hitData.color = tex.SampleLevel(testSampler, vInter.texCoord, 0) * float4(lightInt, 0);


}

void renderMesh(inout HitData hitData, Ray ray, int start, int end)
{
	//declarations
	float3 e1, e2, d, q, s, o, r;
	float a, f, u, v;
	float euro = 0.00001;				//a constant used to avoid determinants close to 0.
	float newT;							//the new t value
	d = ray.direction.xyz;				//we use d for the ray direction
	o = ray.origin.xyz;					//and o for the ray origin
	Triangle hittri, tri;				//triangle indices
	int i;
	int index = start;
	bool hit = false;
	for (i = 0; i < end; i += 3)
	{
		tri.v0 = index++;
		tri.v1 = index++;
		tri.v2 = index++;
		
		e1 = vBuffer[tri.v1].pos - vBuffer[tri.v0].pos;
		e2 = vBuffer[tri.v2].pos - vBuffer[tri.v0].pos;
		q = cross(d, e2);					//cross the direction vector and one of the spanning vectors for a vector q
		a = dot(e1, q);						//create the determinant of the matrix M.  M = [-d, p1 - p0, p2 - p0] 
		if (a > -euro && a < euro)			//if the determinant is too close to 0, abort
			continue;
		//generate u, the "x-coordinate" of the intersection point in the triangle
		f = 1.0 / a;
		s = o - vBuffer[tri.v0].pos;
		u = f*dot(s, q);
		if (u < 0.0)						//compare u to the edge of a triangle
			continue;
		r = cross(s, e1);
		v = f*dot(d, r);					//generate v, the "y-coordinate" of the intersection point in the triangle
											//if (v < 0.0 || u + v > 1.0)			//compare v and u to the edges of the triangle
											//	return;
		newT = f*dot(e2, r);				//says q instead of r in the book, but that is incorrect
											//the following if statement was combined with the previous to reduce branching
		if (v < 0.0 || u + v > 1.0 ||		//compare v and u to the edges of the triangle
			newT > hitData.t || newT < 0)	//if hitData.t is negative, then the intersection is behind us
			continue;
		hitData.t = newT;
		hittri = tri;
		hit = true;
	}
	if (!hit)
		return;

	float3 lightInt = 0.0;
	Vertex vInter = interpolate((ray.origin + ray.direction*hitData.t).xyz, hittri);
	for (i = 0; i < noLights; i++)
	{
		Ray ray;
		ray.origin = vInter.pos;
		ray.direction = normalize(lightPos[i].xyz - vInter.pos);
		lightInt += shadow(ray, length(lightPos[i].xyz - vInter.pos)) * saturate(lightColor_Reach[i].xyz * dot(normalize(lightPos[i].xyz - vInter.pos), vInter.norm)) * (lightColor_Reach[i].w / dot(lightPos[i].xyz - vInter.pos.xyz, lightPos[i].xyz - vInter.pos.xyz));
		//http://brooknovak.wordpress.com/2008/11/13/hlsl-per-pixel-point-light-using-phong-blinn-lighting-model/
		//http://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/
	}
	hitData.color = tex.SampleLevel(testSampler, vInter.texCoord, 0) * float4(lightInt, 0);
}

[numthreads(32, 32, 1)]
void csGeneratePrimaryRays( uint3 threadID : SV_DispatchThreadID )
{
	int i;
	Sphere sphere[10];
	for(i = 0; i < noLights; i++)
	{
		sphere[i].pos = lightPos[i];
		sphere[i].color = float4(lightColor_Reach[i].xyz, 1);
		sphere[i].radius = 25;
	}

	float3 right = cross(camUp.xyz, camDir.xyz);

	float x = (threadID.x / resolution.x) -0.5;
	float y = (threadID.y / resolution.y) -0.5;
	float3 imagePoint = x * right - y * camUp.xyz + camPos.xyz + camDir.xyz;	//calculate the pixels pos in worldspace
																				// for the screenpixel this thread is aimed at
	HitData hitData;
	hitData.t = FLT_MAX;
	hitData.color = defaultColor;
	Ray ray;
	ray.origin = camPos.xyz;
	ray.direction = normalize(imagePoint - ray.origin.xyz);

	hitData.color *= (float3(ray.origin + ray.direction * 100).y - ray.origin.y + 100)*0.01; //create a dark tone to the background floor, to help navigate

	for(i = 0; i < noLights; i++)
	{
		rayVSSphere(hitData,ray,sphere[i]);
	}
	/*Triangle2 tri;
	tri.v0 = float3(0, 10, 50);
	tri.v1 = float3(50, 10, 50);
	tri.v2 = float3(50, 50, 50);
	float oldT = hitData.t;
	rayVSTriangleNoNormals(hitData, tri, ray);*/
	//rayVSTriangle(hitData, ray);
	renderMesh(hitData, ray, 0, noVertices);

	output[threadID.xy] = hitData.color;
}