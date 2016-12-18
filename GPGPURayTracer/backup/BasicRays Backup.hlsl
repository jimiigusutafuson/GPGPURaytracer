//--------------------------------------------------------------------------------------
// BasicRays.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

//definitions
#define defaultColor float4(0.9,0.3,0.4,0);
#define FLT_MAX 3.402823466e+38F;

//samplers
SamplerState samLinear	: register( s0 );

SamplerState testSampler : IMMUTABLE
 {
     Filter = MIN_MAG_MIP_LINEAR;
     AddressU = Wrap;
     AddressV = Wrap;
 };


//textures
Texture2D tex			: register( t0 );

//structs
struct Ray
{
	float3	origin;
	float3	direction;
};

struct Sphere
{
	float3	pos;
	float4	color;
	float	radius;
};

struct HitData
{
	float4	color;
	float	t;
};

struct Vertex
{
	float3 pos;
	float3 norm;
	float2 texCoord;
};

struct Triangle
{
	int v0;
	int v1;
	int v2;
};
struct Triangle2
{
	float3 v0;
	float3 v1;
	float3 v2;
};
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
	int noLights;					//number of light
	float3 lightPadding;
};

RWTexture2D<float4> output : register(u0);
RWStructuredBuffer<Vertex> vBuffer : register(u1);

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

void rayVSSphere(inout HitData hitData, Ray ray, Sphere sphere)
{
	//----------------------------------------working only if sphere is always assumed to have position (0,0,0)
	/*float a = dot(ray.direction.xyz, ray.direction.xyz); //squared length of ray.direction ( ||d||^2 )
	float b = 2 * dot(ray.direction.xyz, ray.origin.xyz);
	float c = dot(ray.origin.xyz, ray.origin.xyz) - (sphere.radius * sphere.radius);
	
	float t1, t2;
	float f = b*b - 4*a*c;
	if(f >= 0.0f)
	{
		f = sqrt(f);
		t1 = -b - f;
		t2 = -b + f;

		hitData.t		= min(t1,t2);
		hitData.color = sphere.color;
	}*/

	//---------------------------------------working only if ray direction vector is normalized
	/*float tnow = FLT_MAX;
	float3 sd = sphere.pos.xyz - ray.origin.xyz;
	float b = dot( ray.direction.xyz, sd );
	float disc = b*b + sphere.radius*sphere.radius - dot(sd,sd);

	if(disc > 0)
		tnow = b - sqrt(disc);

	if( tnow > 0.00001 && hitData.t > tnow )
	{
		hitData.t = tnow;
		hitData.color = sphere.color;
	}*/

	//----------------------------------working only if the direction vector is normalized first (following the Real-Time Rendering 3rd Edition example)
	float r2 = sphere.radius*sphere.radius;						//calculate the radius squared
	float3 l = sphere.pos.xyz - ray.origin.xyz;					//a vector going between the center of the sphere and the origin of the ray
	float lLenSquared = dot(l, l);								//the length of the vector l, squared
	//if(lLenSquared < ray.radius*ray.radius)					//if the length squared is lower than the radius squared, then we know that we're inside the sphere
	float ldProj = dot(l, ray.direction.xyz);					//the projection of the l vector onto the ray direction
	/*if (ldProj < 0 && lLenSquared > r2)
		return;	*/												//sphere is behind ray origin
	float squareDistFromProj = lLenSquared - (ldProj*ldProj);	//squared distance from projection
	if (squareDistFromProj > r2)
		return;													//ray will miss
	float squaredDistFromHit = r2 - squareDistFromProj;
	float distFromHit = sqrt(squaredDistFromHit);
	float t1 = ldProj - distFromHit;							//one of the intersection spots
	//float t2 = ldProj + distFromHit;							//the other intersection spot

	//the prior if statements were combined with the following to reduce branching
	if ((ldProj < 0 && lLenSquared > r2) || t1 > hitData.t)		//check if in front of or behind old hit
		return;
	hitData.t = t1;
	hitData.color = sphere.color;
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

void rayVSTriangleNoNormals(inout HitData hitData, Triangle2 tri2, Ray ray)
{
	//-------------------------going by the book. (no precomputed normals)

	//declarations
	float3 e1, e2, d, q, s, o, r;
	float a, f, u, v;
	float euro = 0.00001;				//a constant used to avoid determinants close to 0.
	float newT;							//the new t value
	d = ray.direction.xyz;				//we use d for the ray direction
	o = ray.origin.xyz;					//and o for the ray origin
	//create vectors spanning the triangle plane
	e1 = tri2.v1 - tri2.v0;
	e2 = tri2.v2 - tri2.v0;
	q = cross(d, e2);					//cross the direction vector and one of the spanning vectors for a vector q
	a = dot(e1, q);						//create the determinant of the matrix M.  M = [-d, p1 - p0, p2 - p0] 
	if (a > -euro && a < euro)			//if the determinant is too close to 0, abort
		return;
	//generate u, the "x-coordinate" of the intersection point in the triangle
	f = 1.0 / a;
	s = o - tri2.v0;
	u = f*dot(s, q);
	if (u < 0.0)						//compare u to the edge of a triangle
		return;
	r = cross(s, e1);
	v = f*dot(d, r);					//generate v, the "y-coordinate" of the intersection point in the triangle
	//if (v < 0.0 || u + v > 1.0)			//compare v and u to the edges of the triangle
	//	return;
	newT = f*dot(e2, r);				//says q instead of r in the book, but that is incorrect
	//the following if statement was combined with the previous to reduce branching
	if (v < 0.0 || u + v > 1.0 ||		//compare v and u to the edges of the triangle
		newT > hitData.t || newT < 0 )	//if hitData.t is negative, then the intersection is behind us
		return;
	hitData.t = newT;
	hitData.color = float4(1, 1, 1, 1); //<------------------------------- remove this later 
}

[numthreads(32, 32, 1)]
void main( uint3 threadID : SV_DispatchThreadID )
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

	//create a dark tone to the background floor, to help navigate
	hitData.color *= (float3(ray.origin + ray.direction * 100).y - ray.origin.y+100)*0.01;

	float closest = FLT_MAX;
	for(i = 0; i < noLights; i++)
	{
		rayVSSphere(hitData,ray,sphere[i]);
	}

	//int index = 0;
	
	//rayVSTriangle(hitData, ray);
	Triangle2 tri;
	tri.v0 = float3(0, 10, 50);
	tri.v1 = float3(50, 10, 50);
	tri.v2 = float3(50, 50, 50);

	float oldT = hitData.t;

	rayVSTriangleNoNormals(hitData, tri, ray);

	output[threadID.xy] = hitData.color;
	//output[threadID.xy] = float4(float3(1,0,0.4)  * (1 - length(threadID.xy - float2(683, 384)) / 400.0f) , 1);
}