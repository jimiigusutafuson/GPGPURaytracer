#include "D3DStructuredBuffer.h"



D3DStructuredBuffer::D3DStructuredBuffer()
{
	buffer = nullptr;
	uav = nullptr;
	srv = nullptr;
	uavStartSlot = -1;
	srvStartSlot = -1;
}


D3DStructuredBuffer::~D3DStructuredBuffer()
{
	if(uav != nullptr)
		uav->Release();
	if (srv != nullptr)
		srv->Release();
	if (buffer != nullptr)
		buffer->Release();
}

bool D3DStructuredBuffer::init(UINT iNumElements, const bool  isCpuWritable, const bool isGpuWritable, const void *pData, UINT byteStride, Graphics *g)
{
	//make sure all buffers are null
	ID3D11Buffer *buffer = NULL;

	//create the basics of the buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = iNumElements * byteStride;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = byteStride;

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
		std::string message = "Buffer cannot be fully accessable both on CPU and GPU";
		MessageBox(NULL, message.c_str(), "D3DStructuredBuffer Error", MB_OK);
		return false; //cannot be fully accessable on both cpu and gpu
	}

	bool temp;
	if (pData == NULL)
		temp = createBuffer(g, &bufferDesc, NULL, 0);
	else
		temp = createBuffer(g, &bufferDesc, pData, iNumElements);
	if(!temp)
		return false;

	if (isGpuWritable)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory((&uavDesc), sizeof(uavDesc));
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.NumElements = iNumElements;
		createUnorderedAccessView(&uavDesc, g);
	}
	else
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.ElementWidth = iNumElements;

		createShaderResourceView(&srvDesc, g);
	}

	return true;
}

bool D3DStructuredBuffer::createUnorderedAccessView(D3D11_UNORDERED_ACCESS_VIEW_DESC *pUAVDesc, Graphics *g)
{
	HRESULT hr;
	hr = g->getDevice()->CreateUnorderedAccessView(buffer, pUAVDesc, &uav);
	if (FAILED(hr))
	{
		std::string message = "Failed to create unordered access view";
		MessageBox(NULL, message.c_str(), "D3DStructuredBuffer Error", MB_OK);
		return false;
	}
	return true;
}

bool D3DStructuredBuffer::createShaderResourceView(D3D11_SHADER_RESOURCE_VIEW_DESC *pSRVDesc, Graphics *g)
{
	HRESULT hr;
	hr = g->getDevice()->CreateShaderResourceView(buffer, pSRVDesc, &srv);
	if (FAILED(hr))
	{
		std::string message = "Failed to create shader resource view";
		MessageBox(NULL, message.c_str(), "D3DStructuredBuffer Error", MB_OK);
		return false;
	}
	return true;
}

bool D3DStructuredBuffer::createBuffer(Graphics *g, D3D11_BUFFER_DESC *pBufferDesc, const void *pData, UINT pDataAmount)
{
	HRESULT hr;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	if (pDataAmount > 0)
	{
		data.pSysMem = pData;
		hr = g->getDevice()->CreateBuffer(pBufferDesc, &data, &buffer);
	}
	else
		hr = g->getDevice()->CreateBuffer(pBufferDesc, NULL, &buffer);

	if (FAILED(hr))
	{
		delete buffer;
		std::string message = "Failed to create buffer";
		MessageBox(NULL, message.c_str(), "D3DStructuredBuffer Error", MB_OK);
		return false;
	}
	return true;
}
void D3DStructuredBuffer::use(Graphics *g)
{
	if (hasSRV())
		g->getDeviceContext()->CSSetShaderResources(srvStartSlot, 1, &srv);
	if (hasUAV())
		g->getDeviceContext()->CSSetUnorderedAccessViews(uavStartSlot, 1, &uav, NULL);
}