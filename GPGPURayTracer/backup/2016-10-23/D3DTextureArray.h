#pragma once
#include "D3DResource.h"
class D3DTextureArray :
	public D3DResource
{
public:
	D3DTextureArray();
	~D3DTextureArray();
	bool loadFromFile(std::vector<std::string> texturePaths, Graphics *g);
	ID3D11Texture2D* g_textureArray;
};

