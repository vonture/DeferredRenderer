#pragma once

#include "PCH.h"
#include "IHasContent.h"

class LightBuffer : public IHasContent
{
private:
	// LightRT =   Light.r  | Light.g   | Light.b   | 
	ID3D11Texture2D* _tex;
	ID3D11ShaderResourceView* _srv;
	ID3D11RenderTargetView* _rtv;

	XMFLOAT3 _ambientColor;
	float _ambientBrightness;

public:
	LightBuffer();

	const XMFLOAT3& GetAmbientColor() const { return _ambientColor; }
	void SetAmbientColor(const XMFLOAT3& color) { _ambientColor = color; }

	float GetAmbientBrightness() const { return _ambientBrightness; }
	void SetAmbientBrightness(float brightness) { _ambientBrightness = brightness; }
	
	ID3D11ShaderResourceView* GetLightSRV() { return _srv; }
	ID3D11RenderTargetView* GetLightRTV() { return _rtv; }
	
	HRESULT Clear(ID3D11DeviceContext* pd3dImmediateContext);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};