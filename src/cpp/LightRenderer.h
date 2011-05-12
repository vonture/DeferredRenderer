#pragma once

#include "Light.h"
#include "ModelInstance.h"
#include "BoundingObjects.h"
#include "Camera.h"
#include "GBuffer.h"
#include "DeviceStates.h"
#include <vector>

template <class T>
class LightRenderer : IHasContent
{
private:
	std::vector<T*> _shadowed, _unshadowed;

	DepthStencilStates _dsStates;
	SamplerStates _samplerStates;
	BlendStates _blendStates;
	RasterizerStates _rasterStates;

protected:
	DepthStencilStates* GetDepthStencilStates()
	{
		return &_dsStates;
	}
	SamplerStates* GetSamplerStates()
	{
		return &_samplerStates;
	}
	BlendStates* GetBlendStates()
	{
		return &_blendStates;
	}
	RasterizerStates* GetRasterizerStates()
	{
		return &_rasterStates;
	}

public:
	void Add(T* light, bool shadowed) 
	{
		if (shadowed)
		{
			_shadowed.push_back(light);
		}
		else
		{
			_unshadowed.push_back(light);
		}
	}

	int GetCount(bool shadowed)
	{
		return shadowed ? _shadowed.size() : _unshadowed.size();
	}

	T* GetLight(int idx, bool shadowed)
    {
        return shadowed ? _shadowed[idx] : _unshadowed[idx];
    }
	
	void Clear()
	{
		_shadowed.clear();
		_unshadowed.clear();
	}
	
	virtual HRESULT RenderShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, std::vector<ModelInstance*> models,
		Camera* camera, BoundingBox* sceneBounds) = 0;
	virtual HRESULT RenderLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
		GBuffer* gBuffer) = 0;

	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
	{
		HRESULT hr;

		V_RETURN(_dsStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
		V_RETURN(_samplerStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
		V_RETURN(_blendStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
		V_RETURN(_rasterStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

		return S_OK; 
	}

	virtual void OnD3D11DestroyDevice()
	{
		_dsStates.OnD3D11DestroyDevice();
		_samplerStates.OnD3D11DestroyDevice();
		_blendStates.OnD3D11DestroyDevice();
		_rasterStates.OnD3D11DestroyDevice();
	}

	virtual HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
	{
		HRESULT hr;

		V_RETURN(_dsStates.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
		V_RETURN(_samplerStates.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
		V_RETURN(_blendStates.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
		V_RETURN(_rasterStates.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

		return S_OK;
	}

	virtual void OnD3D11ReleasingSwapChain()
	{
		_dsStates.OnD3D11ReleasingSwapChain();
		_samplerStates.OnD3D11ReleasingSwapChain();
		_blendStates.OnD3D11ReleasingSwapChain();
		_rasterStates.OnD3D11ReleasingSwapChain();
	}
};
