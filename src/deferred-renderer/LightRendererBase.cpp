#include "PCH.h"
#include "LightRendererBase.h"

LightRendererBase::LightRendererBase()
{
	SetAlphaCutoutEnabled(true);
	SetAlphaThreshold(0.05f);
}

LightRendererBase::~LightRendererBase()
{
}

HRESULT LightRendererBase::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(_dsStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	V_RETURN(_samplerStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	V_RETURN(_blendStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	V_RETURN(_rasterStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

	return S_OK; 
}

void LightRendererBase::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
	_dsStates.OnD3D11DestroyDevice(pContentManager);
	_samplerStates.OnD3D11DestroyDevice(pContentManager);
	_blendStates.OnD3D11DestroyDevice(pContentManager);
	_rasterStates.OnD3D11DestroyDevice(pContentManager);
}

HRESULT LightRendererBase::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(_dsStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_samplerStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_blendStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_rasterStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void LightRendererBase::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{
	_dsStates.OnD3D11ReleasingSwapChain(pContentManager);
	_samplerStates.OnD3D11ReleasingSwapChain(pContentManager);
	_blendStates.OnD3D11ReleasingSwapChain(pContentManager);
	_rasterStates.OnD3D11ReleasingSwapChain(pContentManager);
}