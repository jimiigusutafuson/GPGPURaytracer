#pragma once
#include <d3dcompiler.h>
#include <d3d11.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "DirectxTEX\WICTextureLoader.h"
class Graphics
{
public:
	Graphics();
	~Graphics();

	struct CBuffer
	{
		ID3D11Buffer *cb;
		UINT byteWidth;
	};

	bool									init(HWND *p_hWnd);
	HRESULT									createCBuffer(UINT byteWidth, UINT registerIndex);

	ID3D11Device							*getDevice() { return g_device; }
	IDXGISwapChain							*getSwapChain() { return g_swapChain; }
	ID3D11DeviceContext						*getDeviceContext() { return g_deviceContext; }

	void									setDebugName(ID3D11DeviceChild* child, const std::string& name);
	void									unsetShader(UINT numClassInstance);

	void									updateSubResource(int index, UINT dstSubResource, const D3D11_BOX *pDstBox, const void *pSrcData, UINT srcRowPitch, UINT srcDepthPitch);
	void									swapPresent(UINT syncIntervals, UINT flags);
	void									clearRenderTarget(float x, float y, float z, float w);

private:
	D3D_DRIVER_TYPE							g_driverType;
	D3D_FEATURE_LEVEL						g_featureLevel;
	std::map<UINT, CBuffer>					g_cbuffers;

	ID3D11Device							*g_device;
	IDXGISwapChain							*g_swapChain;
	ID3D11DeviceContext						*g_deviceContext;
	ID3D11UnorderedAccessView				*uavBackBuffer;
};

