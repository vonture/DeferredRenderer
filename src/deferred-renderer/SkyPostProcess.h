#pragma once

#include "PCH.h"
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

	struct SKY_TYPE
	{
		float A, B, C, D, E;
		const WCHAR* Description;
	};

	static const UINT SKY_TYPE_COUNT = 15;
	static const SKY_TYPE SKY_TYPES[SKY_TYPE_COUNT];
	UINT _skyTypeIndex;	

	ID3D11PixelShader* _skyPSs[2][SKY_TYPE_COUNT];

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
	void SetSunWidth(float width) { _sunWidth = max(width, EPSILON); }
	void SetSunEnabled(bool enable) { _enableSun = enable; }
	void SetSunIntensity(float intensity) { _sunIntensity = max(intensity, 0.0f); }

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