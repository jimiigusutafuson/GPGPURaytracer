#include "ComputeShader.h"


HRESULT ComputeShader::compile(_In_ LPCWSTR pFilePath, _In_ LPCSTR pEntryPoint, _In_ ID3D11Device *pDevice, _Outptr_ ID3DBlob **blob)
{
	if (!pFilePath || !pEntryPoint || !pDevice || !blob)
		return E_INVALIDARG;

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

	//#ifdef _DEBUG
	//    flags = D3DCOMPILE_DEBUG;
	//#endif

	LPCSTR profile = (pDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";

	const D3D_SHADER_MACRO defines[] =
	{
		NULL, NULL // add macros here for the shader file
	};

	ID3DBlob *shaderBlob = nullptr;
	ID3DBlob *errorBlob = nullptr;

	HRESULT hr = D3DCompileFromFile(pFilePath,
									defines,
									D3D_COMPILE_STANDARD_FILE_INCLUDE,
									pEntryPoint,
									profile,
									flags,
									0,
									&shaderBlob,
									&errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			MessageBox(NULL, (char*)errorBlob->GetBufferPointer(), "ComputeShader Error", S_OK);
			errorBlob->Release();
		}
		if (shaderBlob)
		{
			shaderBlob->Release();
		}
		return hr;
	}

	*blob = shaderBlob;

	return hr;
}