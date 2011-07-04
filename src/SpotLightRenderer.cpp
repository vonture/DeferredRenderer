#include "SpotLightRenderer.h"

SpotLightRenderer::SpotLightRenderer()
{
}

SpotLightRenderer::~SpotLightRenderer()
{
}

HRESULT SpotLightRenderer::RenderShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, std::vector<ModelInstance*>* models,
	Camera* camera, AxisAlignedBox* sceneBounds)
{
	if (GetCount(true) > 0)
	{
		DXUT_BeginPerfEvent(D3DCOLOR_COLORVALUE(0.0f, 0.0f, 1.0f, 1.0f), L"Spot Light Shadow Maps");

		DXUT_EndPerfEvent();
		return E_FAIL;
	}
	return S_OK;
}

HRESULT SpotLightRenderer::RenderLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera, GBuffer* gBuffer)
{
	if (GetCount(true) + GetCount(false) > 0)
	{
		DXUT_BeginPerfEvent(D3DCOLOR_COLORVALUE(0.0f, 0.0f, 1.0f, 1.0f), L"Spot Lights");

		DXUT_EndPerfEvent();
		return E_FAIL;
	}
	return S_OK;
}

HRESULT SpotLightRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(LightRenderer::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	return S_OK;
}

void SpotLightRenderer::OnD3D11DestroyDevice()
{
	LightRenderer::OnD3D11DestroyDevice();
}

HRESULT SpotLightRenderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(LightRenderer::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}
void SpotLightRenderer::OnD3D11ReleasingSwapChain()
{
	LightRenderer::OnD3D11ReleasingSwapChain();
}