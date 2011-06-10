#pragma once

#include "Defines.h"
#include "PostProcess.h"
#include "FullscreenQuad.h"

struct CB_AO_PROPERTIES
{
};

class AmbientOcclusionPostProcess : public PostProcess
{
private:
	ID3D11Texture2D* _aoTexture;
	ID3D11RenderTargetView* _aoRTV;
	ID3D11ShaderResourceView* _aoSRV;

	ID3D11Texture2D* _downScaleTextures[2];
	ID3D11RenderTargetView* _downScaleRTVs[2];
	ID3D11ShaderResourceView* _downScaleSRVs[2];

	ID3D11Texture2D* _blurTempTexture;
	ID3D11RenderTargetView* _blurTempRTV;
	ID3D11ShaderResourceView* _blurTempSRV;

	ID3D11PixelShader* _aoPS;
	ID3D11PixelShader* _scalePS;
	ID3D11PixelShader* _hBlurPS;
	ID3D11PixelShader* _vBlurPS;
	
	ID3D11Buffer* _aoPropertiesBuffer;

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