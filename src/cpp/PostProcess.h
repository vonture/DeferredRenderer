#pragma once

#include "DXUT.h"
#include "IHasContent.h"
#include "GBuffer.h"
#include "LightBuffer.h"

class PostProcess : public IHasContent
{
public:
	virtual HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, ID3D11DepthStencilView* dstDSV, GBuffer* gBuffer,
		LightBuffer* lightBuffer) = 0;

	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) { return S_OK; }
	virtual void OnD3D11DestroyDevice() { }

	virtual HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) { return S_OK; }
	virtual void OnD3D11ReleasingSwapChain() { }
};