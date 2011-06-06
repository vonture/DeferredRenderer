#pragma once

#include "Defines.h"
#include "PostProcess.h"
#include "FullscreenQuad.h"

struct CB_HDR_PROPERTIES
{
	float AdaptationRate;
	float KeyValue;
	float TimeDelta;
	UINT MipLevels;
};

class HDRPostProcess : public PostProcess
{
private:
	float _timeDelta;
	float _adaptationRate;
	float _keyValue;

	UINT _mipLevels;

	ID3D11Texture2D* _adaptLuminanceTextures[2];
	ID3D11RenderTargetView* _adaptLuminanceRTVs[2];
	ID3D11ShaderResourceView* _adaptLuminanceSRVs[2];

	ID3D11PixelShader* _luminanceMapPS;
	ID3D11PixelShader* _toneMapPS;

	ID3D11Buffer* _hdrPropertiesBuffer;

	FullscreenQuad _fsQuad;

	void swapLuminanceBuffers();

public:
	HDRPostProcess();
	~HDRPostProcess();

	float GetTimeDelta() const { return _timeDelta; }
	void SetTimeDelta(float dt) { _timeDelta = max(dt, 0.0f); }

	float GetAdaptationRate() const { return _adaptationRate; }
	void SetAdaptationRate(float adapt) { _adaptationRate = max(adapt, 0.0f); }

	float GetKeyValue() const { return _keyValue; }
	void SetKeyValue(float key) { _keyValue = max(key, 0.0f); }

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, ID3D11DepthStencilView* dstDSV, GBuffer* gBuffer,
		LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};