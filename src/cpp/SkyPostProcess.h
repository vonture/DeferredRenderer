#pragma once

#include "Defines.h"
#include "PostProcess.h"
#include "FullscreenQuad.h"

struct CB_SKY_PROPERTIES
{
	XMFLOAT3 SkyColor;
	XMFLOAT3 SunColor;
	XMFLOAT3 SunDirection;
	XMFLOAT3 Padding;
};

class SkyPostProcess : public PostProcess
{
private:
	XMVECTOR _skyColor;
	XMVECTOR _sunColor;
	XMVECTOR _sunDirection;

	ID3D11PixelShader* _skyPS;

	ID3D11Buffer* _skyProperties;

	FullscreenQuad _fsQuad;
public:
	SkyPostProcess();
	~SkyPostProcess();

	const XMVECTOR& GetSkyColor() const { return _skyColor; }
	const XMVECTOR& GetSunColor() const { return _sunColor; }
	const XMVECTOR& GetSunDirection() const { return _sunDirection; }

	void SetSkyColor(const XMVECTOR& skyCol) { _skyColor = skyCol; }
	void SetSunColor(const XMVECTOR& sunCol) { _sunColor = sunCol; }
	void SetSunDirection(const XMVECTOR& sunDir) { _sunDirection = sunDir; }

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};