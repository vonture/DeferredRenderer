#include "PCH.h"
#include "HBAOPostProcess.h"
#include "Logger.h"

HBAOPostProcess::HBAOPostProcess()
{
}

HRESULT HBAOPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
                                ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer,ParticleBuffer* pBuffer, LightBuffer* lightBuffer)
{
    BEGIN_EVENT_D3D(L"HBAO");
    END_EVENT_D3D(L"");

    return E_NOTIMPL;
}

HRESULT HBAOPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

    return S_OK;
}

void HBAOPostProcess::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
    PostProcess::OnD3D11DestroyDevice(pContentManager);
}

HRESULT HBAOPostProcess::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                                                 const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

    return S_OK;
}

void HBAOPostProcess::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{
    PostProcess::OnD3D11ReleasingSwapChain(pContentManager);
}