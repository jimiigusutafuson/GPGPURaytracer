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
void Graphics::init(HWND *p_hWnd)
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
		return;
	}

	ID3D11Texture2D *backBuffer;
	hr = g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	if (FAILED(hr))
	{
		MessageBox(NULL, "Error creating backbuffer", "Graphics Error", S_OK);
		return;
	}
	g_uavs.push_back(NULL);
	hr = g_device->CreateUnorderedAccessView(backBuffer, NULL, &g_uavs.back());
	if (FAILED(hr))
	{
		MessageBox(NULL, "Error creating unordered access view", "Graphics Error", S_OK);
		return;
	}



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
			return;
		}
	}
}

void Graphics::clearRenderTarget(float x, float y, float z, float w)
{
	float clearColor[4] = { x,y,z,w };
	g_deviceContext->ClearUnorderedAccessViewFloat(g_uavs[0], clearColor);
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
		MessageBox(NULL, message.c_str(), "GraphicsCore Error", MB_OK);
		return hr;
	}
	g_deviceContext->CSSetConstantBuffers(registerIndex, 1, &g_cbuffers.at(registerIndex).cb);
	return hr;
}

int Graphics::createBuffer(D3D11_BUFFER_DESC *pBufferDesc, const void *pData = NULL, UINT pDataAmount = 0 )
{
	HRESULT hr;
	ID3D11Buffer *vBuffer;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	if (pDataAmount > 0)
		data.pSysMem = pData;
	else
		data.pSysMem = NULL;
	hr = g_device->CreateBuffer(pBufferDesc, &data, &vBuffer);

	if (FAILED(hr))
	{
		delete vBuffer;
		std::string message = "Failed to create buffer";
		MessageBox(NULL, message.c_str(), "GraphicsCore Error", MB_OK);
		return -1;
	}
	g_buffers.push_back(vBuffer);
	return g_buffers.size() - 1; //return the index
}

int Graphics::createUnorderedAccessView(D3D11_UNORDERED_ACCESS_VIEW_DESC *pUAVDesc, UINT pBufferIndex)
{
	HRESULT hr;
	ID3D11UnorderedAccessView *uav;
	hr = g_device->CreateUnorderedAccessView(g_buffers.at(pBufferIndex), pUAVDesc, &uav);
	if (FAILED(hr))
	{
		std::string message = "Failed to create unordered access view";
		MessageBox(NULL, message.c_str(), "GraphicsCore Error", MB_OK);
		return -1;
	}
	g_uavs.push_back(uav);
	return g_uavs.size() - 1; //return the index
}

int Graphics::createShaderResourceView(D3D11_SHADER_RESOURCE_VIEW_DESC *pSRVDesc, UINT pBufferIndex)
{
	HRESULT hr;
	ID3D11ShaderResourceView *srv;
	hr = g_device->CreateShaderResourceView(g_buffers.at(pBufferIndex), pSRVDesc, &srv);
	if (FAILED(hr))
	{
		std::string message = "Failed to create shader resource view";
		MessageBox(NULL, message.c_str(), "GraphicsCore Error", MB_OK);
		return -1;
	}
	g_srvs.push_back(srv);
	return g_srvs.size() - 1; //return the index
}

bool Graphics::createStructuredBuffer(int &outBufferIndex, int &outViewIndex, UINT iNumElements, const bool  isCpuWritable, const bool isGpuWritable, const void *pData, UINT byteStride)
{
	//make sure all buffers are null
	ID3D11Buffer *buffer = nullptr;

	//create the basics of the buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = iNumElements * byteStride;

	if ((!isCpuWritable) && (!isGpuWritable))
	{
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	}
	else if (isCpuWritable && (!isGpuWritable))
	{
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	}
	else if ((!isCpuWritable) && isGpuWritable)
	{
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.BindFlags = (D3D11_BIND_SHADER_RESOURCE |
			D3D11_BIND_UNORDERED_ACCESS);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	}
	else
	{
		return false; //cannot be fully accessable on both cpu and gpu
	}
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = byteStride;


	outBufferIndex = createBuffer(&bufferDesc, pData, iNumElements);
	if (outBufferIndex < 0)
		return false;

	if (isGpuWritable)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory((&uavDesc), sizeof(uavDesc));
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.NumElements = iNumElements;
		outViewIndex = createUnorderedAccessView(&uavDesc, outBufferIndex);
	}
	else
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.ElementWidth = iNumElements;

		outViewIndex = createShaderResourceView(&srvDesc, outBufferIndex);
	}

	return true;
}

void Graphics::setUnorderedAccessViews(UINT startlot, UINT numUAVs, int uavID, const UINT *pUAVInitialCount)
{
	g_deviceContext->CSSetUnorderedAccessViews(startlot, numUAVs, &g_uavs[uavID], pUAVInitialCount);
}
void Graphics::setShaderResourceView(UINT startlot, UINT numSRVs, int srvID)
{
	g_deviceContext->CSSetShaderResources(startlot, numSRVs, &g_srvs[srvID]);
}
void Graphics::unsetShader(UINT numClassInstance)
{
	g_deviceContext->CSSetShader(NULL, NULL, numClassInstance);
}

void Graphics::swapPresent(UINT syncIntervals, UINT flags)
{
	g_swapChain->Present(syncIntervals, flags);
}

void Graphics::free()
{
	//clear the buffer list
	for (unsigned int i = 0; i < g_buffers.size(); i++)
		delete g_buffers.at(i);
	g_buffers.clear();

	//clear the unordered access view list
	for (unsigned int i = 0; i < g_uavs.size(); i++)
		delete g_uavs.at(i);
	g_uavs.clear();
}