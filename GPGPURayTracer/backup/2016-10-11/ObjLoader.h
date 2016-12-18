#pragma once
#include <iostream>
#include <fstream>
#include <d3d11.h>
#include <directxmath.h>
#include <vector>
#include "Model3D.h"

using namespace std;
using namespace DirectX;

class ObjLoader
{
private: //variables
	ifstream			*file;		//obj file
	ifstream			*mtlFile;	//mtl file
	vector<XMFLOAT3>	*vList;		//vertex(position) list
	vector<XMFLOAT3>	*nList;		//normal list
	vector<XMFLOAT2>	*tList;		//texture coordinate list

	//for models with tangent space based normal maps.
	vector<XMFLOAT3>	*tanList;	//tangent list
	vector<XMFLOAT3>	*bitanList;	//bitangent list
	//record of all missing normal values,
	vector<VertexFaceLink>	*missingNormals;
	vector<VertexFaceLink>	*countedNormals;
	int curGroupIndex;				//the current group index

	//temporary storage for character reading
	char controlSymbol;
	char separator;
	float x, y, z;
	int vals[3];

	string path;					//file path
	int vSize;						//final vertex list size
private: //functions
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
public: //functions
	ObjLoader();
	bool loadModel(Model3D *model, const char *path);
	bool loadMesh(vector<MeshVertex> *outVertices, vector<IndexGroup> *outIGroups,
		vector<Material> *outMaterials, vector<string> *outTexturePaths, const char *filename);
	~ObjLoader();
};

