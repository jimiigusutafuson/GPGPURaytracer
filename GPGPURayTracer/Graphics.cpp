#include "Graphics.h"
#include <sstream>

Graphics::Graphics()
{
	g_device			= NULL;
	g_swapChain			= NULL;
	g_deviceContext		= NULL;
	//g_uavScene			= NULL;
	//g_uavVBuffer		= NULL;
}

Graphics::~Graphics()
{
	uavBackBuffer->Release();
	g_swapChain->Release();
	g_deviceContext->Release();
	g_device->Release();
}
bool Graphics::init(HWND *p_hWnd)
{
	HRESULT hr = S_OK;
	
	RECT rc;
	GetClientRect(*p_hWnd, &rc);
	UINT width	= rc.right - rc.left;
	UINT height	= rc.bottom - rc.top;

	UINT deviceFlags = 0;
	#ifdef _DEBUG
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT driverTypeAmount = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		//D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	UINT featureLevelAmount = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
	sd.OutputWindow = (*p_hWnd);
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	bool succeeded = false;
	for (UINT driverTypeIndex = 0; driverTypeIndex < driverTypeAmount && !succeeded; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(	NULL,
											g_driverType,
											NULL,
											deviceFlags,
											featureLevels,
											featureLevelAmount,
											D3D11_SDK_VERSION,
											&sd,
											&g_swapChain,
											&g_device,
											&g_featureLevel,
											&g_deviceContext );
		if (SUCCEEDED(hr))
			succeeded = true;
	}
	if (!succeeded)
	{
		MessageBox(NULL, "Error creating device and swapchain", "Graphics Error", S_OK);
		return false;
	}

	ID3D11Texture2D *backBuffer;
	hr = g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	if (FAILED(hr))
	{
		MessageBox(NULL, "Error creating backbuffer", "Graphics Error", S_OK);
		return false;
	}
	hr = g_device->CreateUnorderedAccessView(backBuffer, NULL, &uavBackBuffer);
	if (FAILED(hr))
	{
		MessageBox(NULL, "Error creating unordered access view", "Graphics Error", S_OK);
		return false;
	}

	g_deviceContext->CSSetUnorderedAccessViews(0, 1, &uavBackBuffer, NULL);


	//check if DirectX 11 is supported
	if (g_device->GetFeatureLevel() < D3D_FEATURE_LEVEL_11_0)
	{
		//check if compute shader on DirectX 10 is supported
		D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS hwopts = { 0 };
		(void)g_device->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &hwopts, sizeof(hwopts));
		if (!hwopts.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
		{
			g_device->Release();
			MessageBox(NULL, "DirectCompute is not supported", "Graphics Error", S_OK);
			return false;
		}
	}

	return true;
}

void Graphics::clearRenderTarget(float x, float y, float z, float w)
{
	float clearColor[4] = { x,y,z,w };
	g_deviceContext->ClearUnorderedAccessViewFloat(uavBackBuffer, clearColor);
}

HRESULT Graphics::createCBuffer(UINT byteWidth, UINT registerIndex)
{
	g_cbuffers[registerIndex].byteWidth = byteWidth;
	g_cbuffers[registerIndex].cb;
	HRESULT hr;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.ByteWidth = byteWidth;
	hr = g_device->CreateBuffer(&bd, NULL, &g_cbuffers.at( registerIndex ).cb );
	if (FAILED(hr))
	{
		std::stringstream numb;
		numb << registerIndex;
		std::string message = "Failed to create constant buffer " + numb.str();
		MessageBox(NULL, message.c_str(), "Graphics Error", MB_OK);
		return hr;
	}
	g_deviceContext->CSSetConstantBuffers(registerIndex, 1, &g_cbuffers.at(registerIndex).cb);
	return hr;
}

void Graphics::updateSubResource(int index, UINT dstSubResource, const D3D11_BOX *pDstBox, const void *pSrcData, UINT srcRowPitch, UINT srcDepthPitch)
{
	g_deviceContext->UpdateSubresource(g_cbuffers[index].cb, dstSubResource, pDstBox, pSrcData, srcRowPitch, srcDepthPitch);
}

void Graphics::unsetShader(UINT numClassInstance)
{
	g_deviceContext->CSSetShader(NULL, NULL, numClassInstance);
}

void Graphics::swapPresent(UINT syncIntervals, UINT flags)
{
	g_swapChain->Present(syncIntervals, flags);
}

void Graphics::setDebugName(ID3D11DeviceChild* child, const std::string& name)
{
	HRESULT hr = child->SetPrivateData(WKPDID_D3DDebugObjectName, name.size(), name.c_str());
	if (FAILED(hr))
	{
		std::stringstream namestr;
		namestr << name;
		std::string message = "Failed to set the debug name: " + namestr.str();
		MessageBox(NULL, message.c_str(), "Graphics Error", MB_OK);
	}
}