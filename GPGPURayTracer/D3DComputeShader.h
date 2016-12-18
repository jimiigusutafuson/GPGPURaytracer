#pragma once
#include <d3d11.h>
#include <vector>
#include "Graphics.h"
#include "DirectxTEX\WICTextureLoader.h"
#include <string>
class D3DComputeShader
{
public:
	D3DComputeShader();
	~D3DComputeShader();
	bool										createComputeShader(const wchar_t *hlslPath, const char *entry, Graphics *graphics, std::vector<std::pair<std::string, int>> pMacros);
	void										dispatch(Graphics *g, UINT x, UINT y, UINT z);
	void										useSampler(Graphics *g, UINT startSlot, UINT numSamples);
	bool										createSampler(D3D11_SAMPLER_DESC desc, Graphics *g);
private:
	//variables
	ID3D11ComputeShader							*computeShader;
	ID3D11SamplerState							*sampler;
	std::vector<std::pair<std::string, int>>	mMacros; //macros
	//functions
	HRESULT										csCompile(_In_ LPCWSTR pFilePath, _In_ LPCSTR pEntryPoint, _In_ ID3D11Device *pDevice, _Outptr_ ID3DBlob **blob);
};