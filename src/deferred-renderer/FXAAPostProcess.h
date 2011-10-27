#pragma once

#include "PCH.h"
#include "PostProcess.h"

class FXAAPostProcess : public PostProcess
{
private:

#ifdef ALL_PRESETS
	static const UINT QUALITY_PRESET_COUNT = 17;
#else
	static const UINT QUALITY_PRESET_COUNT = 3;
#endif
	static const UINT QUALITY_PRESETS[QUALITY_PRESET_COUNT];
	UINT _qualityIndex;

	float _subpixel;
	float _edgeThreshold;
	float _edgeThresholdMin;

	XMFLOAT2 _invSceneSize;

	ID3D11PixelShader* _fxaaPSs[QUALITY_PRESET_COUNT];
	ID3D11Buffer* _propertiesBuffer;

	struct CB_FXAA_PROPERTIES
	{
		XMFLOAT2	InverseSceneSize;
		float		Subpixel;
		float		EdgeThreshold;
		float		EdgeThresholdMin;
		XMFLOAT3	Padding;
	};

public:
	FXAAPostProcess();

	void SetSubpixelAliasingRemoval(float qual) { _subpixel = clamp(qual, 0.0f, 1.0f); }
	float GetSubpixelAliasingRemoval() const { return _subpixel; }

	void SetEdgeThreshold(float threshold) { _edgeThreshold = clamp(threshold, 0.0f, 0.5f); }
	float GetEdgeThreshold() const { return _edgeThreshold; }

	void SetMinimumEdgeThreshold(float threshold) { _edgeThresholdMin = clamp(threshold, 0.0f, 0.0833f); }
	float GetMinimumEdgeThreshold() const { return _edgeThresholdMin; }

	void SetQualityPresetIndex(UINT idx) { _qualityIndex = clamp(idx, 0, QUALITY_PRESET_COUNT - 1); }
	UINT GetQualityPresetIndex() const { return _qualityIndex; }
	UINT GetQualityPreset() const { return QUALITY_PRESETS[_qualityIndex]; }
	UINT GetQualityPresetIndexCount() { return QUALITY_PRESET_COUNT; }

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};