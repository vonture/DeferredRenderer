#pragma once

#include "DXUT.h"

class IHasContent
{
public:
	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext) = 0;	
	virtual void OnD3D11DestroyDevice(void* pUserContext) = 0;

	virtual HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext) = 0;
	virtual void OnD3D11ReleasingSwapChain(void* pUserContext) = 0;
};