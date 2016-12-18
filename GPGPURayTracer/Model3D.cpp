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

	updateWorld();
}


Model3D::~Model3D()
{

}

void Model3D::updateWorld()
{
	XMStoreFloat4x4(&mScale, XMMatrixScalingFromVector(XMLoadFloat3(&scale)));
	XMStoreFloat4x4(&mRot, XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rot)));
	XMStoreFloat4x4(&mTrans, XMMatrixTranslationFromVector(XMLoadFloat3(&pos)));

	XMStoreFloat4x4(&mWorld, XMMatrixMultiply(XMLoadFloat4x4(&mScale), XMLoadFloat4x4(&mRot)));
	XMStoreFloat4x4(&mWorld, XMMatrixMultiply(XMLoadFloat4x4(&mWorld), XMLoadFloat4x4(&mTrans)));
	XMStoreFloat4x4(&mWorldInv, XMMatrixInverse(&XMMatrixDeterminant(XMLoadFloat4x4(&mWorld)), XMLoadFloat4x4(&mWorld)));
}

void Model3D::move(float x, float y, float z)
{
	pos.x += x;
	pos.y += y;
	pos.z += z;
	updateWorld();
}