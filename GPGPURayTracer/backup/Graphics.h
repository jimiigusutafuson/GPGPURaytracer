#pragma once
#include <d3dcompiler.h>
#include <d3d11.h>
#include <iostream>
#include <string>
#include <map>
#include "WICTextureLoader.h"
#include <vector>
class Graphics
{
public:
	Graphics();
	~Graphics() {};

	struct CBuffer
	{
		ID3D11Buffer *cb;
		UINT byteWidth;
	};

	void init(HWND *p_hWnd);
	int createComputeShader( const wchar_t *hlslPath, const char *entry );
	void clearRenderTarget(float x, float y, float z, float w);
	void updateSubResource(int index, UINT dstSubResource, const D3D11_BOX *pDstBox, const void *pSrcData, UINT srcRowPitch, UINT srcDepthPitch)
	{
		g_deviceContext->UpdateSubresource( g_cbuffers[index].cb, dstSubResource, pDstBox, pSrcData, srcRowPitch, srcDepthPitch );
	}
	bool createShaderResourceViewFromFile( std::vector<int> *outSRVIndices, std::vector<std::string> texturePaths )
	{
		
		HRESULT hr;
		for ( unsigned int i = 0; i < texturePaths.size(); i++ )
		{
			ID3D11ShaderResourceView *srv;
			hr = DirectX::CreateWICTextureFromFile( g_device, widenString( texturePaths[i].c_str() ).c_str(), NULL, &srv, 4096 );
			if ( FAILED( hr ) )
			{
				MessageBox( NULL, "Error loading texture from file", "Graphics::createShaderResourceViewFromFile error", S_OK );
				return false;
			}
			outSRVIndices->push_back(g_srvs.size());
			g_srvs.push_back(srv);
		}
	}

	/*creates a new ID3D11Buffer from a buffer description and data.
	pBufferDesc is the description of the buffer. this determines the type of buffer, and usage.
	pData is the raw data to add to the buffer, leave as NULL if the buffer shall be empty.
	pDataAmount is the number of elements represented in pData. Leave as 0 if the buffer shall be empty.
	returns (success) the array index of the newly created buffer.
	returns (failed) -1.*/
	int createBuffer(D3D11_BUFFER_DESC *pBufferDesc, const void *pData, UINT pDataAmount);

	int createUnorderedAccessView(D3D11_UNORDERED_ACCESS_VIEW_DESC *pUAVDesc, UINT pBufferIndex);
	int createShaderResourceView(D3D11_SHADER_RESOURCE_VIEW_DESC *pSRVDesc, UINT pBufferIndex);
	void setUnorderedAccessViews(UINT startlot, UINT numUAVs, int uavID, const UINT *pUAVInitialCount);
	void setShaderResourceView(UINT startlot, UINT numSRVs, int srvID);
	bool createBuffer_UAV(int &outBufferIndex, int &outViewIndex, const void *pData, UINT byteStride, int vertexAmount);
	bool createBuffer_SRV(int &outBufferIndex, int &outViewIndex, const void *pData, UINT byteStride, int vertexAmount);
	void setShader(int index, UINT numClassInstance);
	void unsetShader(UINT numClassInstance);
	void dispatch(UINT threadGroupCountX, UINT threadGroupCountY, UINT threadGroupCountZ);
	void swapPresent(UINT syncIntervals, UINT flags);

	void free();
	HRESULT createCBuffer(UINT byteWidth, UINT registerIndex);
	/* Makes buffer and view generation easier.
	It sets up the basic description variables for the buffer and the view, and automatically chooses whether the view is SRV or UAV.
	---
	int &outBufferIndex: the index of the buffer inside g_buffers,
	int &outViewIndex:			the index of the view in either the g_uavs or g_srvs depending on what view is generated,
	UINT iNumElements:			the number of elements inside the initial data of the buffer,
	const bool isCPUWritable:	if the buffer can be altered from the CPU, then this should be true,
	const bool isGPUWritable:	if the buffer can be altered from the GPU, then this should be true. (this makes the view an unordered access view),
	const void *pData:			the raw initial data to be inserted into the buffer,
	UINT byteStride:			the byte stride of the buffer (e.g. size of vertex struct)
	returns (success) true
	returns (failed) false*/
	bool createStructuredBuffer(int &outBufferIndex, int &outViewIndex, UINT iNumElements, const bool  isCpuWritable, const bool isGpuWritable, const void *pData, UINT byteStride);

private:
	D3D_DRIVER_TYPE							g_driverType;
	D3D_FEATURE_LEVEL						g_featureLevel;
	ID3D11Device							*g_device;
	IDXGISwapChain							*g_swapChain;
	ID3D11DeviceContext						*g_deviceContext;
	//ID3D11UnorderedAccessView				*g_uavScene; //					TODO put in g_uavs
	//ID3D11ComputeShader						*g_computeShader;
	std::map<UINT, CBuffer>					g_cbuffers;
	std::vector<ID3D11Buffer*>				g_buffers;
	std::vector<ID3D11UnorderedAccessView*>	g_uavs;
	std::vector<ID3D11ShaderResourceView*>	g_srvs;
	std::vector<ID3D11ComputeShader*>		g_computeShaders;

	//computeShader
	HRESULT csCompile(_In_ LPCWSTR pFilePath, _In_ LPCSTR pEntryPoint, _In_ ID3D11Device *pDevice, _Outptr_ ID3DBlob **blob);

	//convert from string to wstring
	std::wstring widenString( const std::string& str );

	//convert from wstring to string
	std::string narrowString( const std::wstring& str );
};

