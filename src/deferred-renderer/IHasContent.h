#pragma once

#include "PCH.h"
#include "ContentManager.h"

class IHasContent
{
public:
	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, 
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) = 0;	
	virtual void OnD3D11DestroyDevice(ContentManager* pContentManager) = 0;

	virtual HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, 
		IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) = 0;
	virtual void OnD3D11ReleasingSwapChain(ContentManager* pContentManager) = 0;
};