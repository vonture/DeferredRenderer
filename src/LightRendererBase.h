#pragma once

#include "PCH.h"
#include "IHasContent.h"
#include "DeviceStates.h"
#include "ModelInstance.h"
#include "Camera.h"
#include "GBuffer.h"

class LightRendererBase : public IHasContent
{
private:	
	DepthStencilStates _dsStates;
	SamplerStates _samplerStates;
	BlendStates _blendStates;
	RasterizerStates _rasterStates;

protected:
	DepthStencilStates* GetDepthStencilStates() { return &_dsStates; }
	SamplerStates* GetSamplerStates() { return &_samplerStates; }
	BlendStates* GetBlendStates() { return &_blendStates; }
	RasterizerStates* GetRasterizerStates() { return &_rasterStates; }

	virtual UINT GetMaxShadowedLights() const = 0;

public:
	virtual HRESULT RenderShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, std::vector<ModelInstance*>* models,
		Camera* camera, AxisAlignedBox* sceneBounds) = 0;
	virtual HRESULT RenderLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
		GBuffer* gBuffer) = 0;

	virtual UINT GetCount() = 0;
	virtual UINT GetCount(bool shadowed) = 0;

	virtual void Clear() = 0;

	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	virtual void OnD3D11DestroyDevice();

	virtual HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	virtual void OnD3D11ReleasingSwapChain();
};