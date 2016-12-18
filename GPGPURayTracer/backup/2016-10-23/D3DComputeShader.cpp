#include "D3DComputeShader.h"
#include <sstream>


D3DComputeShader::D3DComputeShader()
{
	computeShader = nullptr;
	sampler = nullptr;
}


D3DComputeShader::~D3DComputeShader()
{
	if(computeShader != nullptr)
		computeShader->Release();
	if(sampler != nullptr)
		sampler->Release();
}

bool D3DComputeShader::createComputeShader(const wchar_t *hlslPath, const char *entry, Graphics *graphics, std::vector<std::pair<std::string, int>> pMacros)
{
	mMacros = pMacros;

	HRESULT hr = S_OK;
	ID3DBlob *blob = nullptr;
	hr = csCompile(hlslPath, entry, graphics->getDevice(), &blob);
	if (FAILED(hr))
	{
		graphics->getDevice()->Release();
		MessageBox(NULL, "Shader compilation failed", "createComputeShader Error", S_OK);
		return false;
	}

	hr = graphics->getDevice()->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &computeShader);

	blob->Release();

	if (FAILED(hr))
	{
		graphics->getDevice()->Release();
		MessageBox(NULL, "Shader creation failed", "createComputeShader Error", S_OK);
		return false;
	}
#ifdef _DEBUG
	graphics->setDebugName(computeShader, "ComputeShader");
#endif
	return true;
}

void D3DComputeShader::dispatch(Graphics *g, UINT x, UINT y, UINT z)
{
	g->getDeviceContext()->CSSetShader(computeShader, NULL, 0);
	g->getDeviceContext()->Dispatch(x, y, z);
}

HRESULT D3DComputeShader::csCompile(_In_ LPCWSTR pFilePath, _In_ LPCSTR pEntryPoint, _In_ ID3D11Device *pDevice, _Outptr_ ID3DBlob **blob)
{
	if (!pFilePath || !pEntryPoint || !pDevice || !blob)
		return E_INVALIDARG;

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG
	flags = D3DCOMPILE_DEBUG;
#endif

	LPCSTR profile = (pDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";

	D3D10_SHADER_MACRO defines[10] = {
		"BLOCK_SIZE_X",	"2",
		"BLOCK_SIZE_Y",	"1",
		"BLOCK_SIZE_Z",	"1",
		"N",			"0" ,
		NULL,			NULL,
		NULL,			NULL,
		NULL,			NULL,
		NULL,			NULL,
		NULL,			NULL,
		NULL,			NULL,
	};
	//insert the macro data into defines
	std::vector<std::string> str(mMacros.size());
	for (unsigned int i = 0; i < mMacros.size(); i++)
	{
		std::stringstream ss;
		ss << mMacros[i].second;
		str[i] = ss.str();

		defines[i].Name = mMacros[i].first.data();
		defines[i].Definition = str[i].data();
	}

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

void D3DComputeShader::useSampler(Graphics *g, UINT startSlot, UINT numSamples)
{
	g->getDeviceContext()->CSSetSamplers(startSlot, numSamples, &sampler);
}

bool D3DComputeShader::createSampler(D3D11_SAMPLER_DESC desc, Graphics *g)
{
	HRESULT hr;
	hr = g->getDevice()->CreateSamplerState(&desc, &sampler);
	if (FAILED(hr))
	{
		MessageBox(NULL, "Failed to create sampler state", "Graphics Error", S_OK);
		return false;
	}
#ifdef _DEBUG
	g->setDebugName(sampler, "sampler");
#endif
	return true;
}