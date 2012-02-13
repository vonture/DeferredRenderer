#pragma once

#include "PCH.h"
#include "IHasContent.h"
#include "GBuffer.h"
#include "ParticleBuffer.h"
#include "LightBuffer.h"
#include "DeviceStates.h"
#include "FullscreenQuad.h"
#include "Camera.h"

class PostProcess : public IHasContent
{
private:
	DepthStencilStates _dsStates;
	SamplerStates _samplerStates;
	BlendStates _blendStates;
	RasterizerStates _rasterStates;
	FullscreenQuad _fsQuad;
	bool _isAdditive;

protected:
	DepthStencilStates* GetDepthStencilStates()	{ return &_dsStates; }
	SamplerStates* GetSamplerStates() { return &_samplerStates; }
	BlendStates* GetBlendStates() { return &_blendStates; }
	RasterizerStates* GetRasterizerStates() { return &_rasterStates; }
	Quad* GetFullScreenQuad() { return &_fsQuad; }

	void SetIsAdditive(bool additive) { _isAdditive = additive; }

public:
	PostProcess() : _isAdditive(false) { }
	virtual ~PostProcess() { }

	bool GetIsAdditive() const { return _isAdditive; }

	virtual HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src, 
		ID3D11RenderTargetView* dst, Camera* camera, GBuffer* gBuffer, ParticleBuffer* pBuffer,
		LightBuffer* lightBuffer) = 0;

	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	virtual void OnD3D11DestroyDevice(ContentManager* pContentManager);

	virtual HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	virtual void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};