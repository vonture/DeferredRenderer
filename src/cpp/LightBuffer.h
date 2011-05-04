#pragma once

#include "DXUT.h"
#include "DeferredBuffer.h"

class LightBuffer : public DeferredBuffer
{
private:
	// LightRT =   Light.r  | Light.g   | Light.b   | 
	ID3D11Texture2D* _texture;
	ID3D11ShaderResourceView* _shaderResourceView;
	ID3D11RenderTargetView* _renderTargetView;

public:
	LightBuffer()
		: _texture(NULL), _shaderResourceView(NULL), _renderTargetView(NULL)
	{		
	}

	const ID3D11ShaderResourceView* GetShaderResourceView(int idx)
	{
		if (idx != 0)
		{
			return NULL;
		}

		return _shaderResourceView;
	}

	ID3D11ShaderResourceView*const* GetShaderResourceViews()
	{
		return &_shaderResourceView;
	}

	int GetShaderResourceCount()
	{
		return 1;
	}
	
	const ID3D11RenderTargetView* GetRenderTargetView(int idx)
	{
		if (idx != 0)
		{
			return NULL;
		}

		return _renderTargetView;
	}
	
	ID3D11RenderTargetView*const* GetRenderTargetViews()
	{
		return &_renderTargetView;
	}

	int GetRenderTargetViewCount()
	{
		return 1;
	}

	const ID3D11DepthStencilView* GetDepthStencilView()
	{
		return NULL;
	}

	const ID3D11DepthStencilState* GetDepthStencilState()
	{
		return NULL;
	}

	int GetCount()
	{
		return 1;
	}

	HRESULT GSSetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx);
	HRESULT VSSetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx);
	HRESULT PSSetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx);

	HRESULT GSUnsetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx);
	HRESULT VSUnsetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx);
	HRESULT PSUnsetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx);

	HRESULT Clear(ID3D11DeviceContext* pd3dImmediateContext);

	HRESULT SetRenderTargets(ID3D11DeviceContext* pd3dImmediateContext);
	HRESULT UnsetRenderTargets(ID3D11DeviceContext* pd3dImmediateContext);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};