#include "Model3D.h"
#include "Graphics.h"

Model3D::Model3D( XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale )
{
	this->pos = pos;
	this->rot = rot;
	this->scale = scale;

	iGroups = vector<IndexGroup>();
	materials = vector<Material>();
	texturePaths = vector<string>();
	textures = vector<ID3D11ShaderResourceView*>();
	vertices = vector<MeshVertex>();
}

bool Model3D::loadModel( string filePath )
{
	if ( !ObjLoader::getInstance()->LoadMesh( &vertices, &iGroups, &materials, filePath.c_str(), &texturePaths ) )
		return false;

	//Graphics::getInstance()->createShaderResourceViewFromFile();
}

void Model3D::useMaterial()
{
	//Graphics::getInstance()->immediateContext->CSSetShaderResources( 0, textures.size(), &textures[0] );
}

Model3D::~Model3D()
{
}
