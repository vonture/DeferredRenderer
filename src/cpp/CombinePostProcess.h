#pragma once

#include "DXUT.h"
#include "PostProcess.h"
#include "FullscreenQuad.h"

class CombinePostProcess : public PostProcess
{
private:
	ID3D11PixelShader* _pixelShader;
	ID3D11SamplerState* _sampler;
	FullscreenQuad _fsQuad;

public:
	CombinePostProcess();
	~CombinePostProcess();

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, ID3D11DepthStencilView* dstDSV, GBuffer* gBuffer,
		LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};