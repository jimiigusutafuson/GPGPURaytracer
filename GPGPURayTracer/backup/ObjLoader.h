#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <d3d11.h>
#include <directxmath.h>

using namespace std;
using namespace DirectX;
#pragma once
struct Material
{
	string name;
	float ns;		//specular intensity (0-1000)
	float ni;		//optical density (index of refraction)
	float tr;		//Transparency a.k.a  d (dissolved)
	int illum;		//Illumination type
	XMFLOAT3 tf;	//transmission filter (decides what colors get to pass through the material)
	XMFLOAT3 ka;	//ambient
	XMFLOAT3 kd;	//diffuse
	XMFLOAT3 ks;	//specular lightning. black = off.
	XMFLOAT3 ke;	//emissive. The self-illumination color.

	int	map_ka;		//index of ambient map
	int	map_kd;		//index of diffuse map
	int map_ks;		//index of specular map
	int map_ke;		//index of emissive map
	int map_ns;		//index of specular intensity map
	int map_tr;		//index of transparency map
	int map_bump;	//index of bump map
	int map_disp;	//index of displacement map
	int map_decal;	//index of stencil decal map
	int map_normal;	//index of normal map

	Material()
	{
		ns = 0;
		ni = 0;
		tr = 0;
		tf.x = 1;
		tf.y = 1;
		tf.z = 1;
		ka.x = 0.08f;
		ka.y = 0.08f;
		ka.z = 0.08f;
		kd.x = 0.5f;
		kd.y = 0.5f;
		kd.z = 0.5f;
		ks.x = 0.2f;
		ks.y = 0.2f;
		ks.z = 0.2f;
		ke.x = 0;
		ke.y = 0;
		ke.z = 0;

		map_ka = -1;
		map_kd = -1;
		map_ks = -1;
		map_ke = -1;
		map_ns = -1;
		map_tr = -1;
		map_bump = -1;
		map_disp = -1;
		map_decal = -1;
		map_normal = -1;
	};
};
struct VertexFaceLink
{
	unsigned int vertexIndex;
	unsigned int normalIndex;
};
struct IndexGroup
{
	string name;
	unsigned int iStart;
	unsigned int iEnd;
	Material *material;
	bool hasNormals;
};

struct MeshVertex
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 texC;

	MeshVertex( XMFLOAT3 pos, XMFLOAT3 normal, XMFLOAT2 texC )
	{
		this->pos = pos;
		this->normal = normal;
		this->texC = texC;
	}
	MeshVertex()
	{
		pos = XMFLOAT3( 0, 0, 0 );
		normal = XMFLOAT3( 0, 0, 0 );
		texC = XMFLOAT2( 0, 0 );
	}
};


class ObjLoader
{
private:
	static ObjLoader	*instance;
	ObjLoader();
	//obj file
	ifstream			*file;
	//mtl file
	ifstream			*mtlFile;
	//vertex(position) list
	vector<XMFLOAT3>	*vList;
	//normal list
	vector<XMFLOAT3>	*nList;
	//texture coordinate list
	vector<XMFLOAT2>	*tList;

	//for models with tangent space based normal maps.
	//tangent list
	vector<XMFLOAT3>	*tanList;
	//bitangent list
	vector<XMFLOAT3>	*bitanList;
	//record of all missing normal values,
	vector<VertexFaceLink>	*missingNormals;
	vector<VertexFaceLink>	*countedNormals;
	//the current group index
	int curGroupIndex;

	//temporary storage for character reading
	char controlSymbol;
	char separator;
	float x, y, z;
	int vals[3];

	//file path
	string path;
	int vSize; //final vertex list size

	bool findFile( const char *path );
	void addMap( int *map, vector<string> *texturePaths );

	void readVertex();
	void readFace( vector<MeshVertex> *vertices );
	void readMaterial( vector<Material> *materials, vector<string> *texturePaths );
	void readMtlFile( string filename, vector<Material> *materials, vector<string> *texturePaths );
	void readGroup( vector<IndexGroup> *outIGroups );
	void readUse( vector<Material> *materials, vector<IndexGroup> *outIGroups );
	void readSmooth();
	void generateNormals( vector<MeshVertex> *vertices );
public:
	static ObjLoader	*getInstance()
	{
		if ( !instance )
			instance = new ObjLoader();
		return instance;
	}
	bool LoadMesh( vector<MeshVertex> *outVertices, vector<IndexGroup> *outIGroups,
		vector<Material> *outMaterials, const char *filename, vector<string> *texturePaths );
	~ObjLoader();
};

