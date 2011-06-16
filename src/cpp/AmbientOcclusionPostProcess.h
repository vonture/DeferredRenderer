#pragma once

#include "Defines.h"
#include "PostProcess.h"
#include "FullscreenQuad.h"

#define SSAO_SAMPLE_COUNT 16

struct CB_AO_PROPERTIES
{
	XMFLOAT4X4 ViewProjection;
	XMFLOAT4X4 InverseViewProjection;
	float SampleRadius;
	float DistanceScale;
	float BlurSigma;
	float GaussianNumerator;
	float CameraNearClip;
	float CameraFarClip;
	XMFLOAT2 Padding;
};

struct CB_AO_SAMPLE_DIRECTIONS
{
	XMFLOAT4 SampleDirections[SSAO_SAMPLE_COUNT];
};

class AmbientOcclusionPostProcess : public PostProcess
{
private:
	float _sampleRadius;
	float _distanceScale;
	float _blurSigma;

	ID3D11Texture2D* _aoTexture;
	ID3D11RenderTargetView* _aoRTV;
	ID3D11ShaderResourceView* _aoSRV;

	ID3D11Texture2D* _downScaleTextures[2];
	ID3D11RenderTargetView* _downScaleRTVs[2];
	ID3D11ShaderResourceView* _downScaleSRVs[2];

	ID3D11Texture2D* _blurTempTexture;
	ID3D11RenderTargetView* _blurTempRTV;
	ID3D11ShaderResourceView* _blurTempSRV;

	static const int RANDOM_TEXTURE_SIZE = 16;
	ID3D11Texture2D* _randomTexture;
	ID3D11ShaderResourceView* _randomSRV;

	XMFLOAT4 _sampleDirections[SSAO_SAMPLE_COUNT];

	ID3D11PixelShader* _aoPS;
	ID3D11PixelShader* _scalePS;
	ID3D11PixelShader* _hBlurPS;
	ID3D11PixelShader* _vBlurPS;
	
	ID3D11Buffer* _aoPropertiesBuffer;
	ID3D11Buffer* _sampleDirectionsBuffer;

	FullscreenQuad _fsQuad;

public:
	AmbientOcclusionPostProcess();
	~AmbientOcclusionPostProcess();
	
	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};