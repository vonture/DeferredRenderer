#pragma once

#include "Defines.h"
#include "PostProcess.h"

class SkyPostProcess : public PostProcess
{
private:
	XMFLOAT3 _skyColor;
	XMFLOAT3 _sunColor;
	XMFLOAT3 _sunDirection;
	float _sunWidth;
	float _sunIntensity;
	bool _enableSun;

	ID3D11PixelShader* _sunEnabledPS;
	ID3D11PixelShader* _sunDisabledPS;

	ID3D11Buffer* _skyProperties;

	struct CB_SKY_PROPERTIES
	{
		float SunWidth;
		float SunIntensity;
		_DECLSPEC_ALIGN_16_ XMFLOAT3 SkyColor;
		_DECLSPEC_ALIGN_16_ XMFLOAT3 SunColor;	
		_DECLSPEC_ALIGN_16_ XMFLOAT3 SunDirection;
		_DECLSPEC_ALIGN_16_ XMFLOAT3 CameraPosition;	
		_DECLSPEC_ALIGN_16_ XMFLOAT4X4 InverseViewProjection;
	};
	
public:
	SkyPostProcess();
	~SkyPostProcess();

	const XMFLOAT3& GetSkyColor() const { return _skyColor; }
	const XMFLOAT3& GetSunColor() const { return _sunColor; }
	const XMFLOAT3& GetSunDirection() const { return _sunDirection; }
	float GetSunWidth() const { return _sunWidth; }
	bool GetSunEnabled() const { return _enableSun; }
	float GetSunIntensity() const { return _sunIntensity; }

	void SetSkyColor(const XMFLOAT3& skyCol) { _skyColor = skyCol; }
	void SetSunColor(const XMFLOAT3& sunCol) { _sunColor = sunCol; }
	void SetSunDirection(const XMFLOAT3& sunDir);
	void SetSunWidth(float width) { _sunWidth = max(width, 0.00001f); }
	void SetSunEnabled(bool enable) { _enableSun = enable; }
	void SetSunIntensity(float intensity) { _sunIntensity = max(intensity, 0.0f); }

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};