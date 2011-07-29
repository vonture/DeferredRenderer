#pragma once

#include "Defines.h"
#include "Lights.h"
#include "ModelInstance.h"
#include "Camera.h"
#include "GBuffer.h"
#include "DeviceStates.h"
#include <vector>

template <class T>
class LightRenderer : IHasContent
{
private:
	static const UINT MAX_LIGHTS = 1024;

	UINT _shadowedCount;
	T* _shadowed[MAX_LIGHTS];
	
	UINT _unshadowedCount;
	T* _unshadowed[MAX_LIGHTS];

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

	virtual UINT GetMaxShadowedLights() const = 0;

public:
	LightRenderer()
		: _shadowedCount(0), _unshadowedCount(0)
	{
	}

	~LightRenderer()
	{
	}

	void Add(T* light, bool shadowed) 
	{
		if (shadowed && _shadowedCount < max(GetMaxShadowedLights(), MAX_LIGHTS))
		{
			_shadowed[_shadowedCount] = light;
			_shadowedCount++;
		}
		else if (_unshadowedCount < MAX_LIGHTS)
		{
			_unshadowed[_unshadowedCount] = light;
			_unshadowedCount++;
		}
	}

	UINT GetCount()
	{
		return _shadowedCount + _unshadowedCount;
	}

	UINT GetCount(bool shadowed)
	{
		return shadowed ? _shadowedCount : _unshadowedCount;
	}

	T* GetLight(UINT idx)
    {
		return (idx >= _shadowedCount) ? _unshadowed[idx] :  _shadowed[idx - _shadowedCount];
    }

	T* GetLight(UINT idx, bool shadowed)
    {
        return shadowed ? _shadowed[idx] : _unshadowed[idx];
    }
	
	void Clear()
	{
		_shadowedCount = 0;
		_unshadowedCount = 0;
	}
	
	virtual HRESULT RenderShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, std::vector<ModelInstance*>* models,
		Camera* camera, AxisAlignedBox* sceneBounds) = 0;
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
