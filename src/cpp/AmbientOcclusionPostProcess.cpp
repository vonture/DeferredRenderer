#include "AmbientOcclusionPostProcess.h"

AmbientOcclusionPostProcess::AmbientOcclusionPostProcess()
{
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

	_fsQuad.OnD3D11ReleasingSwapChain();
}