//--------------------------------------------------------------------------------------
// ProperObjLoader.h
// Author: Jimmy Gustafsson
//--------------------------------------------------------------------------------------
#pragma once
#include <vector>
#include "Model3D.h"
using namespace std;
#define TRIANGLE_MESH (0)
#define SQUARE_MESH (1)

struct Index
{
	int pos;
	int tex;
	int norm;
};
class ProperObjLoader
{
public:
	ProperObjLoader();
	~ProperObjLoader();

	bool readObjLine(char *lineHeader, int size, FILE *file, string pathNoName, bool forceTriangleMesh);
	bool readMtlLine(char *lineHeader, int size, FILE *file);
	bool loadObj(const char *path, bool forceTriangleMesh = false);
	bool loadMtl(const char *path, string pathNoName);
	void generateVertexBufferNoIndices(vector<MeshVertex> *vertices);
	void clear();
public:
	//obj
	vector< Index > g_indices;
	vector< XMFLOAT3 > g_vertices;
	vector< XMFLOAT2 > g_UVs;
	vector< XMFLOAT3 > g_normals;
	vector< int > g_smoothing; //Smoothing group. has one value for each triangle/square
	vector< Material > g_materials;
	vector< string > g_materialNames;
	vector< IndexGroup > g_indexGroups;
	vector< string > g_indexGroupNames;
	//mtl
	vector< string > g_map_kaPaths;
	vector< string > g_map_kdPaths;
	vector< string > g_map_ksPaths;
	vector< string > g_map_kePaths;
	vector< string > g_map_nsPaths;
	vector< string > g_map_trPaths;
	vector< string > g_map_bumpPaths;
	vector< string > g_map_dispPaths;
	vector< string > g_map_decalPaths;
	vector< string > g_map_normalPaths;
private:
	int typeFlag;
	int currentS;
	int currentG;
	int currentM;
	string pathNoName;
	FILE *objFile;
};

