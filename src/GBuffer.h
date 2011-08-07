#pragma once

#include "PCH.h"
#include "DeferredBuffer.h"

class GBuffer : public DeferredBuffer
{
private:
	// RT0 =       Diffuse.r	| Diffuse.g		| Diffuse.b		| Specular Intensity
    // RT1 =       Normal.x		| Normal.y		| Normal.z		| Specular Power
    // RT2 =       Emissive.r	| Emissive.g	| Emissive.b	| Material ID
    // RT3 =       Depth		|				|				|
	ID3D11Texture2D* _textures[4];
	ID3D11ShaderResourceView* _shaderResourceViews[4];
	ID3D11RenderTargetView* _renderTargetViews[3];
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11DepthStencilView* _readonlyDepthStencilView;

public:
	GBuffer()
	{
		for (int i = 0; i < 4; i++)
		{
			_textures[i] = NULL;
			_shaderResourceViews[i] = NULL;
		}

		for (int i = 0; i < 3; i++)
		{
			_renderTargetViews[i] = NULL;
		}

		_depthStencilView = NULL;
		_readonlyDepthStencilView = NULL;
	}
	
	ID3D11ShaderResourceView* GetShaderResourceView(int idx)
	{
		if (idx < 0 || idx >= 4)
		{
			return NULL;
		}

		return _shaderResourceViews[idx];
	}

	ID3D11ShaderResourceView*const* GetShaderResourceViews()
	{
		return &_shaderResourceViews[0];
	}

	int GetShaderResourceCount()
	{
		return 4;
	}
	
	ID3D11RenderTargetView* GetRenderTargetView(int idx)
	{
		if (idx < 0 || idx >= 3)
		{
			return NULL;
		}

		return _renderTargetViews[idx];
	}
	
	ID3D11RenderTargetView*const* GetRenderTargetViews()
	{
		return &_renderTargetViews[0];
	}

	int GetRenderTargetViewCount()
	{
		return 3;
	}

	ID3D11DepthStencilView* GetDepthStencilView()
	{
		return _depthStencilView;
	}

	ID3D11DepthStencilView* GetReadOnlyDepthStencilView() 
	{
		return _readonlyDepthStencilView;
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

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};