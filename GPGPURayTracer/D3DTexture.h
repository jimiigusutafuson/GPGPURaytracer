#pragma once
#include "D3DResource.h"
class D3DTexture :
	public D3DResource
{
public:
	D3DTexture();
	~D3DTexture();
	bool loadFromFile(std::string texturePath, Graphics *g);
	void setShaderResourceView(Graphics *g);
};

