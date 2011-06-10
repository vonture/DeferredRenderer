#include "AmbientOcclusionPostProcess.h"

AmbientOcclusionPostProcess::AmbientOcclusionPostProcess()
	: _aoTexture(NULL), _aoRTV(NULL), _aoSRV(NULL), _blurTempTexture(NULL), _blurTempRTV(NULL), 
	  _blurTempSRV(NULL), _aoPS(NULL), _scalePS(NULL), _hBlurPS(NULL), _vBlurPS(NULL),
	  _aoPropertiesBuffer(NULL)
{
	for (UINT i = 0; i < 2; i++)
	{
		_downScaleTextures[i] = NULL;
		_downScaleRTVs[i] = NULL;
		_downScaleSRVs[i] = NULL;
	}
}

AmbientOcclusionPostProcess::~AmbientOcclusionPostProcess()
{
}

HRESULT AmbientOcclusionPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
	ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer)
{
	return E_FAIL;
}

HRESULT AmbientOcclusionPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_fsQuad.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	return S_OK;
}

void AmbientOcclusionPostProcess::OnD3D11DestroyDevice()
{
	PostProcess::OnD3D11DestroyDevice();
	
	SAFE_RELEASE(_aoPS);
	SAFE_RELEASE(_scalePS);
	SAFE_RELEASE(_hBlurPS);
	SAFE_RELEASE(_vBlurPS);

	SAFE_RELEASE(_aoPropertiesBuffer);

	_fsQuad.OnD3D11DestroyDevice();
}

HRESULT AmbientOcclusionPostProcess::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice,
	IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));	
	V_RETURN(_fsQuad.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void AmbientOcclusionPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();

	SAFE_RELEASE(_aoTexture);
	SAFE_RELEASE(_aoRTV);
	SAFE_RELEASE(_aoSRV);

	for (UINT i = 0; i < 2; i++)
	{
		SAFE_RELEASE(_downScaleTextures[i]);
		SAFE_RELEASE(_downScaleRTVs[i]);
		SAFE_RELEASE(_downScaleSRVs[i]);
	}

	SAFE_RELEASE(_blurTempTexture);
	SAFE_RELEASE(_blurTempRTV);
	SAFE_RELEASE(_blurTempSRV);

	_fsQuad.OnD3D11ReleasingSwapChain();
}