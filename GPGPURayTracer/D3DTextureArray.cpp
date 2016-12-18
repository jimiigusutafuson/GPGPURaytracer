#include "D3DTextureArray.h"

D3DTextureArray::D3DTextureArray()
{
}


D3DTextureArray::~D3DTextureArray()
{
	if(g_textureArray)
		g_textureArray->Release();
}

bool D3DTextureArray::loadFromFile(std::vector<std::string> texturePaths, Graphics *g)
{
	if (texturePaths.size() <= 0)
		return false;

	HRESULT hr;
	//initialize official texture array
	D3D11_TEXTURE2D_DESC dstex;
	ZeroMemory(&dstex, sizeof(dstex));
	dstex.Width = 1024;
	dstex.Height = 1024;
	dstex.MipLevels = 1;
	dstex.ArraySize = texturePaths.size();
	dstex.SampleDesc.Count = 1;
	dstex.SampleDesc.Quality = 0;
	dstex.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dstex.Usage = D3D11_USAGE_DEFAULT;
	dstex.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	dstex.CPUAccessFlags = 0;
	dstex.MiscFlags = 0;
	g->getDevice()->CreateTexture2D(&dstex, NULL, (ID3D11Texture2D**)&g_resource);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Texture2DArray.MostDetailedMip = 0;
	desc.Texture2DArray.MipLevels = 1;
	desc.Texture2DArray.FirstArraySlice = 0;
	desc.Texture2DArray.ArraySize = texturePaths.size();;
	hr = g->getDevice()->CreateShaderResourceView((ID3D11Texture2D*)g_resource, &desc, &g_srv);
	if (FAILED(hr))
	{
		MessageBox(NULL, "Error creating official texture array", "D3DTextureArray::loadFromFile error", S_OK);
		return false;
	}

	//initialize temporary texture array
	ID3D11Texture2D* tempTextureArray;
	//array desc
	D3D11_TEXTURE2D_DESC arrayDesc;
	ZeroMemory(&arrayDesc, sizeof(arrayDesc));
	arrayDesc.Width = 1024;
	arrayDesc.Height = 1024;
	arrayDesc.MipLevels = 1;
	arrayDesc.ArraySize = texturePaths.size();
	arrayDesc.SampleDesc.Count = 1;
	arrayDesc.SampleDesc.Quality = 0;
	arrayDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	arrayDesc.Usage = D3D11_USAGE_DEFAULT;
	arrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	arrayDesc.CPUAccessFlags = 0;
	arrayDesc.MiscFlags = 0;
	hr = g->getDevice()->CreateTexture2D(&arrayDesc, NULL, &tempTextureArray);
	if (FAILED(hr))
	{
		MessageBox(NULL, "Error creating temporary texture array", "D3DTextureArray::loadFromFile error", S_OK);
		return false;
	}

	for (unsigned int i = 0; i < texturePaths.size(); i++)
	{
		hr = DirectX::CreateWICTextureFromFile(g->getDevice(), widenStringArray(texturePaths)[i].c_str(), (ID3D11Resource**)&tempTextureArray, NULL, 4096);
		if (FAILED(hr))
		{
			std::string output = "Error loading texture [ " + texturePaths[i] + " ]from file";
			MessageBox(NULL, output.c_str(), "D3DTextureArray::loadFromFile error", S_OK);
			return false;
		}
		g->getDeviceContext()->CopySubresourceRegion(g_resource, D3D11CalcSubresource(0, i, 1), 0, 0, 0, tempTextureArray, 0, NULL);
	}
	tempTextureArray->Release();
	return true;
}