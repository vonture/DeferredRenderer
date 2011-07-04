#pragma once

#include "Defines.h"
#include "IHasContent.h"
#include "GBuffer.h"
#include "LightBuffer.h"
#include "DeviceStates.h"
#include "Camera.h"

class PostProcess : public IHasContent
{
private:
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
	virtual HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dst, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer) = 0;

	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	virtual void OnD3D11DestroyDevice();

	virtual HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	virtual void OnD3D11ReleasingSwapChain();
};