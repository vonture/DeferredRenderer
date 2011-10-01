#pragma once

#include "PCH.h"
#include "IHasContent.h"

class DeferredBuffer : public IHasContent
{
public:
	DeferredBuffer() { }
	virtual ~DeferredBuffer() { }

	virtual ID3D11ShaderResourceView* GetShaderResourceView(int idx) = 0;
	virtual ID3D11ShaderResourceView*const* GetShaderResourceViews() = 0;	
	virtual int GetShaderResourceCount() = 0;

	virtual ID3D11RenderTargetView* GetRenderTargetView(int idx) = 0;
	virtual ID3D11RenderTargetView*const* GetRenderTargetViews() = 0;
	virtual int GetRenderTargetViewCount() = 0;

	virtual ID3D11DepthStencilView* GetDepthStencilView() = 0;
	virtual ID3D11DepthStencilView* GetReadOnlyDepthStencilView() = 0;

	virtual HRESULT GSSetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx) = 0;
	virtual HRESULT VSSetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx) = 0;
	virtual HRESULT PSSetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx) = 0;

	virtual HRESULT GSUnsetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx) = 0;
	virtual HRESULT VSUnsetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx) = 0;
	virtual HRESULT PSUnsetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx) = 0;

	virtual HRESULT SetRenderTargets(ID3D11DeviceContext* pd3dImmediateContext, ID3D11DepthStencilView* dsv) = 0;
	virtual HRESULT SetRenderTargetsAndDepthStencil(ID3D11DeviceContext* pd3dImmediateContext) = 0;

	virtual HRESULT UnsetRenderTargets(ID3D11DeviceContext* pd3dImmediateContext, ID3D11DepthStencilView* dsv) = 0;
	virtual HRESULT UnsetRenderTargetsAndDepthStencil(ID3D11DeviceContext* pd3dImmediateContext) = 0;

	virtual HRESULT Clear(ID3D11DeviceContext* pd3dImmediateContext) = 0;

	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) { return S_OK; }	
	virtual void OnD3D11DestroyDevice() { }

	virtual HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) { return S_OK; }
	virtual void OnD3D11ReleasingSwapChain() { }
};
