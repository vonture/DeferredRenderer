#pragma once

#include "DXUT.h"
#include "IHasContent.h"

class PostProcess : public IHasContent
{
private:
public:
	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext);	
	void OnD3D11DestroyDevice(void* pUserContext);

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
	void OnD3D11ReleasingSwapChain(void* pUserContext);
};