#pragma once

#include "PCH.h"
#include "IHasContent.h"

class GBuffer : public IHasContent
{
private:
	// RT0 =       Diffuse.r	| Diffuse.g		| Diffuse.b		| Specular Intensity
    // RT1 =       Normal.x		| Normal.y		| Normal.z		| Specular Power
    // RT2 =       Velocity.r	| Velocity.g	| Velocity.b	| Material ID
    // RT3 =       Depth		|				|				| Sky Visibility
	ID3D11Texture2D* _textures[4];
	ID3D11ShaderResourceView* _srvs[4];
	ID3D11RenderTargetView* _rtvs[3];
	ID3D11DepthStencilView* _dsv;
	ID3D11DepthStencilView* _rodsv;

public:
	GBuffer();

	ID3D11ShaderResourceView* GetDiffuseSRV() { return _srvs[0]; }
	ID3D11ShaderResourceView* GetNormalSRV() { return _srvs[1]; }
	ID3D11ShaderResourceView* GetVelocitySRV() { return _srvs[2]; }
	ID3D11ShaderResourceView* GetDepthSRV() { return _srvs[3]; }

	ID3D11RenderTargetView* GetDiffuseRTV() { return _rtvs[0]; }
	ID3D11RenderTargetView* GetNormalRTV() { return _rtvs[1]; }
	ID3D11RenderTargetView* GetVelocityRTV() { return _rtvs[2]; }

	ID3D11DepthStencilView* GetDepthDSV() { return _dsv; }
	ID3D11DepthStencilView* GetReadOnlyDepthDSV() { return _rodsv; }
	
	HRESULT Clear(ID3D11DeviceContext* pd3dImmediateContext);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};