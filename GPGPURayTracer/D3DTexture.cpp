#include "D3DTexture.h"
#include "DirectxTEX\WICTextureLoader.h"

D3DTexture::D3DTexture()
{
}


D3DTexture::~D3DTexture()
{
	//g_resource->Release();
	//g_srv->Release();
}

bool D3DTexture::loadFromFile(std::string texturePath, Graphics *g)
{
	HRESULT hr;
	hr = DirectX::CreateWICTextureFromFile(g->getDevice(), widenString(texturePath).c_str(), &g_resource, &g_srv, 4096);
	if (FAILED(hr))
	{
		MessageBox(NULL, "Error loading texture from file", "Graphics::createShaderResourceViewFromFile error", S_OK);
		return false;
	}
	return true;
}


void D3DTexture::setShaderResourceView( Graphics *g)
{
	g->getDeviceContext()->CSSetShaderResources(g_srvStartSlot, 1, &g_srv);
}