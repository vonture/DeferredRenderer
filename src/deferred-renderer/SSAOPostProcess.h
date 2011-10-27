#pragma once

#include "PCH.h"
#include "PostProcess.h"

class SSAOPostProcess : public PostProcess
{
private:
	float _sampleRadius;
	float _blurSigma;
	float _samplePower;
	bool _halfRes;
	UINT _sampleCountIndex;

	XMFLOAT2 _invSceneSize;

#ifdef ALL_PRESETS
	static const UINT NUM_SSAO_SAMPLE_COUNTS = 7;
	static const UINT SSAO_SAMPLE_COUNT_MAX = 256;
#else
	static const UINT NUM_SSAO_SAMPLE_COUNTS = 2;
	static const UINT SSAO_SAMPLE_COUNT_MAX = 64;
#endif
	static const UINT SSAO_SAMPLE_COUNTS[NUM_SSAO_SAMPLE_COUNTS];
	
	
	XMFLOAT4 _sampleDirections[SSAO_SAMPLE_COUNT_MAX];

	ID3D11Texture2D* _aoTexture;
	ID3D11RenderTargetView* _aoRTV;
	ID3D11ShaderResourceView* _aoSRV;

	ID3D11Texture2D* _blurTempTexture;
	ID3D11RenderTargetView* _blurTempRTV;
	ID3D11ShaderResourceView* _blurTempSRV;

	static const UINT RANDOM_TEXTURE_SIZE = 32;	
	ID3D11Texture2D* _randomTexture;
	ID3D11ShaderResourceView* _randomSRV;

	ID3D11PixelShader* _aoPSs[NUM_SSAO_SAMPLE_COUNTS];
	ID3D11PixelShader* _hBlurPS[2];
	ID3D11PixelShader* _vBlurPS[2];
	ID3D11PixelShader* _compositePS[2];
	
	ID3D11Buffer* _aoPropertiesBuffer;
	ID3D11Buffer* _sampleDirectionsBuffers[NUM_SSAO_SAMPLE_COUNTS];

	struct CB_AO_PROPERTIES
	{
		XMFLOAT4X4 ViewProjection;
		XMFLOAT4X4 InverseViewProjection;
		float SampleRadius;
		float BlurSigma;
		float GaussianNumerator;
		float CameraNearClip;
		float CameraFarClip;
		float SamplePower;
		XMFLOAT2 InverseSceneSize;
	};

public:
	SSAOPostProcess();

	float GetSampleRadius() const { return _sampleRadius; }
	void SetSampleRadius(float radius) { _sampleRadius = max(radius, 0.0f); }

	float GetBlurSigma() const { return _blurSigma; }
	void SetBlurSigma(float sigma) { _blurSigma = max(sigma, EPSILON); }

	float GetSamplePower() const { return _samplePower; }
	void SetSamplePower(float power) { _samplePower = max(power, EPSILON); }

	bool GetHalfResolution() const { return _halfRes; }
	void SetHalfResolution(bool val) { _halfRes = val; }

	UINT GetSampleCount() const { return SSAO_SAMPLE_COUNTS[_sampleCountIndex]; }
	
	UINT GetSampleCountIndex() const { return _sampleCountIndex; }
	void SetSampleCountIndex(UINT index) { _sampleCountIndex = clamp(index, 0, NUM_SSAO_SAMPLE_COUNTS - 1); }
	UINT GetNumSampleCountIndices() const { return NUM_SSAO_SAMPLE_COUNTS; }

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};