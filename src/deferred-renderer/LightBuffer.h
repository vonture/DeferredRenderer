#pragma once

#include "PCH.h"
#include "DeferredBuffer.h"

class LightBuffer : public DeferredBuffer
{
private:
	// LightRT =   Light.r  | Light.g   | Light.b   | 
	ID3D11Texture2D* _texture;
	ID3D11ShaderResourceView* _shaderResourceView;
	ID3D11RenderTargetView* _renderTargetView;

	XMFLOAT3 _ambientColor;

public:
	LightBuffer()
		: _texture(NULL), _shaderResourceView(NULL), _renderTargetView(NULL), _ambientColor(0.0f, 0.0f, 0.0f)
	{		
	}

	const XMFLOAT3& GetAmbientColor() const { return _ambientColor; }
	void SetAmbientColor(const XMFLOAT3& color) { _ambientColor = color; }

	ID3D11ShaderResourceView* GetShaderResourceView(int idx)
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
	
	ID3D11RenderTargetView* GetRenderTargetView(int idx)
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

	ID3D11DepthStencilView* GetDepthStencilView()
	{
		return NULL;
	}

	ID3D11DepthStencilView* GetReadOnlyDepthStencilView() 
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

	HRESULT SetRenderTargets(ID3D11DeviceContext* pd3dImmediateContext, ID3D11DepthStencilView* dsv);
	HRESULT SetRenderTargetsAndDepthStencil(ID3D11DeviceContext* pd3dImmediateContext);

	HRESULT UnsetRenderTargets(ID3D11DeviceContext* pd3dImmediateContext, ID3D11DepthStencilView* dsv);
	HRESULT UnsetRenderTargetsAndDepthStencil(ID3D11DeviceContext* pd3dImmediateContext);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};