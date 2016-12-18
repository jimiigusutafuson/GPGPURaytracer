#pragma once
#include "ObjLoader.h"

class Model3D
{
private:
	XMFLOAT4X4 mWorld;

	XMFLOAT3 pos, rot, scale;

	vector<MeshVertex>	vertices;
	vector<Material>	materials;
	vector<IndexGroup>	iGroups;
public:
	vector<string>						texturePaths;
	vector<ID3D11ShaderResourceView*>	textures;
public:
	const XMFLOAT4X4			*getWorld() { return &mWorld; }
	const vector<MeshVertex>	*getVertices() { return &vertices; }
	bool						loadModel( string filePath );
	void						useMaterial();

	Model3D( XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale );
	~Model3D();
};

