//--------------------------------------------------------------------------------------
// Intersections.hlsl
// DirectX 11 Shader Model 5.0
//--------------------------------------------------------------------------------------

//simple sphere intersection
void rayVSSphere(inout HitData hitData, Ray ray, Sphere sphere)
{
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

//simple triangle intersection
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
		newT > hitData.t || newT < 0)	//if hitData.t is negative, then the intersection is behind us
		return;
	hitData.t = newT;
	hitData.color = float4(1, 1, 1, 1); //<------------------------------- remove this later 
}