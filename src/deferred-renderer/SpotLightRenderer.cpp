#include "PCH.h"
#include "SpotLightRenderer.h"
#include "Logger.h"

SpotLightRenderer::SpotLightRenderer()
{
}

SpotLightRenderer::~SpotLightRenderer()
{
}

HRESULT SpotLightRenderer::RenderGeometryShadowMaps(ID3D11DeviceContext* pd3dImmediateContext,
	std::vector<ModelInstance*>* models, Camera* camera, AxisAlignedBox* sceneBounds)
{
	if (GetCount(true) > 0)
	{
		BEGIN_EVENT_D3D(L"Spot Light Shadow Maps");

		END_EVENT_D3D(L"");
		return E_NOTIMPL;
	}
	return S_OK;
}

HRESULT SpotLightRenderer::RenderGeometryLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
	GBuffer* gBuffer)
{
	if (GetCount(true) + GetCount(false) > 0)
	{
		BEGIN_EVENT_D3D(L"Spot Lights");

		END_EVENT_D3D(L"");
		return E_NOTIMPL;
	}
	return S_OK;
}

HRESULT SpotLightRenderer::RenderParticleLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
	ParticleBuffer* gBuffer )
{
	if (GetCount(true) + GetCount(false) > 0)
	{
		BEGIN_EVENT_D3D(L"Spot Lights");

		END_EVENT_D3D(L"");
		return E_NOTIMPL;
	}
	return S_OK;
}

HRESULT SpotLightRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(LightRenderer::OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

	return S_OK;
}

void SpotLightRenderer::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
	LightRenderer::OnD3D11DestroyDevice(pContentManager);
}

HRESULT SpotLightRenderer::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, 
	IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(LightRenderer::OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void SpotLightRenderer::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{
	LightRenderer::OnD3D11ReleasingSwapChain(pContentManager);
}