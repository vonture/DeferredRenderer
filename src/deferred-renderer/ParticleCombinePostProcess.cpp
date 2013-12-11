#include "PCH.h"
#include "ParticleCombinePostProcess.h"
#include "Logger.h"

ParticleCombinePostProcess::ParticleCombinePostProcess()
    : _pixelShader(NULL)
{
    SetIsAdditive(true);
}

HRESULT ParticleCombinePostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
                                           ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, ParticleBuffer* pBuffer, LightBuffer* lightBuffer)
{
    BEGIN_EVENT_D3D(L"Particle Combine");

    HRESULT hr;

    pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, NULL);

    ID3D11ShaderResourceView* combineSRVs[2] =
    {
        pBuffer->GetDiffuseSRV(),
        lightBuffer->GetParticleLightSRV(),
    };

    pd3dImmediateContext->PSSetShaderResources(0, 2, combineSRVs);

    ID3D11SamplerState* sampler = GetSamplerStates()->GetPointClamp();
    pd3dImmediateContext->PSSetSamplers(0, 1, &sampler);

    pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);

    float blendFactor[4] = {1, 1, 1, 1};
    pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetAlphaBlend(), blendFactor, 0xFFFFFFFF);

    Quad* fsQuad = GetFullScreenQuad();

    V_RETURN(fsQuad->Render(pd3dImmediateContext, _pixelShader->PixelShader));

    // Null the SRVs
    ID3D11ShaderResourceView* NULLSRVs[2] = { NULL, NULL };
    pd3dImmediateContext->PSSetShaderResources(0, 2, NULLSRVs);

    END_EVENT_D3D(L"");

    return S_OK;
}

HRESULT ParticleCombinePostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pContentManager,pBackBufferSurfaceDesc));

    PixelShaderOptions psOpts =
    {
        "PS_Combine",        // const char* EntryPoint;
        NULL,                // D3D_SHADER_MACRO* Defines;
        "P-buffer combine",    // const char* DebugName;
    };

    V_RETURN(pContentManager->LoadContent(pd3dDevice, L"PBufferCombine.hlsl", &psOpts, &_pixelShader));

    return S_OK;
}

void ParticleCombinePostProcess::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
    PostProcess::OnD3D11DestroyDevice(pContentManager);

    SAFE_CM_RELEASE(pContentManager, _pixelShader);
}

HRESULT ParticleCombinePostProcess::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                                                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

    return S_OK;
}

void ParticleCombinePostProcess::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{
    PostProcess::OnD3D11ReleasingSwapChain(pContentManager);
}