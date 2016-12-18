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
	textureIndices = vector<int>();
	vertices = vector<MeshVertex>();

	buffer.srvStartlot = 0;
	textures.srvStartSlot = 1;
}


void Model3D::useMaterial(Graphics *g)
{
	//Graphics::getInstance()->immediateContext->CSSetShaderResources( 0, textures.size(), &textures[0] );
	textures.setShaderResourceView(g);
	buffer.use(g);
}

bool Model3D::initBufferAndViews(Graphics *g)
{
	if (!textures.loadFromFile(texturePaths, g))
	{
		MessageBox(0, "Error creating texture and/or view", 0, 0);
		return false;
	}
	if (!buffer.init(vertices.size(), false, false, &(vertices)[0], sizeof(MeshVertex), g))
	{
		MessageBox(0, "Error creating structured buffer and/or view", 0, 0);
		return false;
	}
	return true;
}

Model3D::~Model3D()
{

}