#pragma once

#include "Defines.h"
#include "PostProcess.h"
#include "FullscreenQuad.h"

struct CB_HDR_PROPERTIES
{
	float Tau;
	float KeyValue;
	float TimeDelta;
	UINT MipLevels;
	float BloomThreshold;
	float BloomMagnitude;
	float BloomBlurSigma;
    float GaussianNumerator;	
};

class HDRPostProcess : public PostProcess
{
private:
	float _timeDelta;
	float _tau;
	float _keyValue;
	float _bloomThreshold;
	float _bloomMagnitude;
	float _bloomBlurSigma;

	UINT _lumMapSize;

	UINT _mipLevels;
	
	ID3D11Texture2D* _lumTextures[2];
	ID3D11RenderTargetView* _lumRTVs[2];
	ID3D11ShaderResourceView* _lumSRVs[2];
	
	ID3D11Texture2D* _downScaleTextures[3];
	ID3D11RenderTargetView* _downScaleRTVs[3];
	ID3D11ShaderResourceView* _downScaleSRVs[3];
	
	ID3D11Texture2D* _blurTempTexture;
	ID3D11RenderTargetView* _blurTempRTV;
	ID3D11ShaderResourceView* _blurTempSRV;

	ID3D11ShaderResourceView* _colorGradeSRV;

	ID3D11PixelShader* _luminanceMapPS;
	ID3D11PixelShader* _toneMapPS;
	ID3D11PixelShader* _scalePS;
	ID3D11PixelShader* _thresholdPS;
	ID3D11PixelShader* _hBlurPS;
	ID3D11PixelShader* _vBlurPS;

	ID3D11Buffer* _hdrPropertiesBuffer;

	FullscreenQuad _fsQuad;

	void swapLuminanceBuffers();

public:
	HDRPostProcess();
	~HDRPostProcess();

	float GetTimeDelta() const { return _timeDelta; }
	void SetTimeDelta(float dt) { _timeDelta = max(dt, 0.0f); }

	float GetTau() const { return _tau; }
	void SetTau(float tau) { _tau = clamp(tau, 0.0f, 1.0f); }

	float GetKeyValue() const { return _keyValue; }
	void SetKeyValue(float key) { _keyValue = max(key, 0.0f); }

	float GetBloomThreshold() const { return _bloomThreshold; }
	void SetBloomThreshold(float thresh) { _bloomThreshold = clamp(thresh, 0.0f, 1.0f); }

	float GetBloomMagnitude() const { return _bloomMagnitude; }
	void SetBloomMagnitude(float mag) { _bloomMagnitude = max(mag, 0.0f); }

	float GetBloomBlurSigma() const { return _bloomBlurSigma; }
	void SetBloomBlurSigma(float sigma) { _bloomBlurSigma = max(sigma, 0.0f); }

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};