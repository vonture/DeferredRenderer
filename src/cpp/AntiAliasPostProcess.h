#pragma once

#include "Defines.h"
#include "PostProcess.h"
#include "FullscreenQuad.h"

class AntiAliasPostProcess : public PostProcess
{
private:
	UINT _quality;

	ID3D11PixelShader* _aaPS;

	FullscreenQuad _fsQuad;

public:
	AntiAliasPostProcess();
	~AntiAliasPostProcess();

	UINT GetQuality() const { return _quality; }
	void SetQuality(UINT qual) { _quality = clamp(qual, 0, 5); }

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};