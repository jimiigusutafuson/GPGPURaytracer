#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>



class ComputeShader
{
public:
	ComputeShader() {};
	~ComputeShader() {};
	HRESULT compile(_In_ LPCWSTR pFilePath, _In_ LPCSTR pEntryPoint, _In_ ID3D11Device *pDevice, _Outptr_ ID3DBlob **blob);
};

