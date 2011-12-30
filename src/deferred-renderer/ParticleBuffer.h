#pragma once

#include "PCH.h"
#include "IHasContent.h"

class ParticleBuffer : public IHasContent
{
private:
	// RT0 =       Diffuse.r	| Diffuse.g		| Diffuse.b
	// RT1 =       Normal.x		| Normal.y		| Normal.z
	ID3D11Texture2D* _textures[2];
	ID3D11ShaderResourceView* _srvs[2];
	ID3D11RenderTargetView* _rtvs[2];

public:
	ParticleBuffer();

	ID3D11ShaderResourceView* GetDiffuseSRV() { return _srvs[0]; }
	ID3D11ShaderResourceView* GetNormalSRV() { return _srvs[1]; }

	ID3D11RenderTargetView* GetDiffuseRTV() { return _rtvs[0]; }
	ID3D11RenderTargetView* GetNormalRTV() { return _rtvs[1]; }

	HRESULT Clear(ID3D11DeviceContext* pd3dImmediateContext);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
		IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};