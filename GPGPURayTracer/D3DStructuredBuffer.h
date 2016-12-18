#pragma once
#include "Graphics.h"
class D3DStructuredBuffer
{
	//variables
public:
	UINT						srvStartSlot;
	UINT						uavStartSlot;
private:
	ID3D11Buffer				*buffer;
	int							bufferSize;
	ID3D11UnorderedAccessView	*uav;
	ID3D11ShaderResourceView	*srv;

	//functions
public:
	D3DStructuredBuffer();
	~D3DStructuredBuffer();
	bool						 init(UINT iNumElements, const bool  isCpuWritable, const bool isGpuWritable, const void *pData, UINT byteStride, Graphics *g);
	void						 use(Graphics *g);

	bool						hasUAV()		{ return uav != nullptr; }
	bool						hasSRV()		{ return srv != nullptr; }
	int							getBufferSize()	{ return bufferSize; }
	ID3D11UnorderedAccessView	*getUAVIndex()	{ return uav; }
	ID3D11ShaderResourceView	*getSRVIndex()	{ return srv; }
private:
	bool						createUnorderedAccessView(D3D11_UNORDERED_ACCESS_VIEW_DESC *pUAVDesc, Graphics *g);
	bool						createShaderResourceView(D3D11_SHADER_RESOURCE_VIEW_DESC *pSRVDesc, Graphics *g);
	bool						createBuffer(Graphics *g, D3D11_BUFFER_DESC *pBufferDesc, const void *pData = NULL, UINT pDataAmount = 0);
};

