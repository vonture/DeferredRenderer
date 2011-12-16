#pragma once

#include "PCH.h"
#include "PostProcess.h"

class SkyPostProcess : public PostProcess
{
private:
	XMFLOAT3 _skyColor;
	float _skyBrightness;

	XMFLOAT3 _sunColor;
	XMFLOAT3 _sunDirection;
	float _sunWidth;
	float _sunBrightness;
	bool _enableSun;

	struct SKY_TYPE
	{
		float A, B, C, D, E;
		const WCHAR* Description;
	};

#ifdef ALL_PRESETS
	static const UINT SKY_TYPE_COUNT = 15;
#else
	static const UINT SKY_TYPE_COUNT = 2;
#endif
	static const SKY_TYPE SKY_TYPES[SKY_TYPE_COUNT];
	UINT _skyTypeIndex;	

	ID3D11PixelShader* _skyPSs[2][SKY_TYPE_COUNT];

	ID3D11Buffer* _skyProperties;

	struct CB_SKY_PROPERTIES
	{
		XMFLOAT3 SunDirection;
		float SunWidth;
		XMFLOAT3 SunColor;
		float SunBrightness;
		XMFLOAT3 SkyColor;
		float SkyBrightness;
		XMFLOAT3 CameraPosition;
		float Padding;
		XMFLOAT4X4 InverseViewProjection;
	};
	
public:
	SkyPostProcess();

	const XMFLOAT3& GetSkyColor() const { return _skyColor; }
	float GetSkyBrightness() const { return _skyBrightness; }
	const XMFLOAT3& GetSunColor() const { return _sunColor; }
	const XMFLOAT3& GetSunDirection() const { return _sunDirection; }
	float GetSunWidth() const { return _sunWidth; }
	bool GetSunEnabled() const { return _enableSun; }
	float GetSunBrightness() const { return _sunBrightness; }

	void SetSkyColor(const XMFLOAT3& skyCol) { _skyColor = skyCol; }
	void SetSkyBrightness(float skyBrightness) { _skyBrightness = skyBrightness; }
	void SetSunColor(const XMFLOAT3& sunCol) { _sunColor = sunCol; }
	void SetSunDirection(const XMFLOAT3& sunDir);
	void SetSunWidth(float width) { _sunWidth = max(width, EPSILON); }
	void SetSunEnabled(bool enable) { _enableSun = enable; }
	void SetSunBrightness(float intensity) { _sunBrightness = max(intensity, 0.0f); }

	void SetSkyTypeIndex(UINT idx) { _skyTypeIndex = clamp(idx, 0, SKY_TYPE_COUNT - 1); }
	UINT GetSkyTypeIndex() const { return _skyTypeIndex; }
	const WCHAR* GetSkyTypeDescription() const { return SKY_TYPES[_skyTypeIndex].Description; }
	UINT GetSkyTypeIndexCount() const { return SKY_TYPE_COUNT; }

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};