#pragma once

#include "PCH.h"

class IHasContent
{
public:
	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) = 0;	
	virtual void OnD3D11DestroyDevice() = 0;

	virtual HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) = 0;
	virtual void OnD3D11ReleasingSwapChain() = 0;
};