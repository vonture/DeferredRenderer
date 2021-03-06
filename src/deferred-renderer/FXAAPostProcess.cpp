#include "PCH.h"
#include "FXAAPostProcess.h"
#include "Logger.h"
#include "PixelShaderLoader.h"

const UINT FXAAPostProcess::QUALITY_PRESETS[QUALITY_PRESET_COUNT] =
{
#ifdef ALL_PRESETS
    10, 11, 12, 13, 14, 15,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    39
#else
    12
#endif
};

FXAAPostProcess::FXAAPostProcess()
    : _propertiesBuffer(NULL)
{
    for (UINT i = 0; i < QUALITY_PRESET_COUNT; i++)
    {
        _fxaaPSs[i] = NULL;
    }

    // Default values
    SetSubpixelAliasingRemoval(0.75f);
    SetEdgeThreshold(0.166f);
    SetMinimumEdgeThreshold(0.0f);

#ifdef ALL_PRESETS
    SetQualityPresetIndex(2);
#else
    SetQualityPresetIndex(0);
#endif
}

HRESULT FXAAPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
                                ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, ParticleBuffer* pBuffer, LightBuffer* lightBuffer)
{
    BEGIN_EVENT_D3D(L"FXAA");

    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    // Map the properties and set them
    V_RETURN(pd3dImmediateContext->Map(_propertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    CB_FXAA_PROPERTIES* properties = (CB_FXAA_PROPERTIES*)mappedResource.pData;

    properties->InverseSceneSize = _invSceneSize;
    properties->Subpixel = _subpixel;
    properties->EdgeThreshold = _edgeThreshold;
    properties->EdgeThresholdMin = _edgeThresholdMin;

    pd3dImmediateContext->Unmap(_propertiesBuffer, 0);

    pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_propertiesBuffer);

    // Set the sampler
    ID3D11SamplerState* samplers[1] = { GetSamplerStates()->GetLinearClamp() };

    pd3dImmediateContext->PSSetSamplers(0, 1, samplers);

    // Set the other states
    pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);

    float blendFactor[4] = {1, 1, 1, 1};
    pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

    // Set the render target
    pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, NULL);

    // Set the resources
    pd3dImmediateContext->PSSetShaderResources(0, 1, &src);

    // Render the quad
    Quad* fsQuad = GetFullScreenQuad();

    V_RETURN(fsQuad->Render(pd3dImmediateContext, _fxaaPSs[_qualityIndex]->PixelShader));

    // Unset the resources
    ID3D11ShaderResourceView* ppSRVNULL[1] = { NULL };
    pd3dImmediateContext->PSSetShaderResources(0, 1, ppSRVNULL);

    END_EVENT_D3D(L"");
    return S_OK;
}

HRESULT FXAAPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

    // Load the shaders
    char psDebugName[256];
    char qualPresetString[6];
    D3D_SHADER_MACRO qualityMacros[] =
    {
        { "FXAA_QUALITY__PRESET", qualPresetString },
        NULL,
    };

    PixelShaderOptions psOpts =
    {
        "PS_FXAA",        // const char* EntryPoint;
        qualityMacros,    // D3D_SHADER_MACRO* Defines;
        psDebugName,    // const char* DebugName;
    };

    for (UINT i = 0; i < QUALITY_PRESET_COUNT; i++)
    {
        sprintf_s(qualPresetString, "%i", QUALITY_PRESETS[i]);
        sprintf_s(psDebugName, "FXAA (quality preset = %s)", qualPresetString);

        V_RETURN(pContentManager->LoadContent(pd3dDevice, L"FXAA.hlsl", &psOpts, &_fxaaPSs[i]));
    }

    // Create the buffers
    D3D11_BUFFER_DESC bufferDesc =
    {
        sizeof(CB_FXAA_PROPERTIES), //UINT ByteWidth;
        D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
        D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
        D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
        0, //UINT MiscFlags;
        0, //UINT StructureByteStride;
    };

    V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_propertiesBuffer));
    V_RETURN(SetDXDebugName(_propertiesBuffer, "FXAA properties buffer"));

    _invSceneSize.x = 1.0f / pBackBufferSurfaceDesc->Width;
    _invSceneSize.y = 1.0f / pBackBufferSurfaceDesc->Height;

    return S_OK;
}

void FXAAPostProcess::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
    PostProcess::OnD3D11DestroyDevice(pContentManager);

    for (UINT i = 0; i < QUALITY_PRESET_COUNT; i++)
    {
        SAFE_CM_RELEASE(pContentManager, _fxaaPSs[i]);
    }

    SAFE_RELEASE(_propertiesBuffer);
}

HRESULT FXAAPostProcess::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                                                 const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

    _invSceneSize.x = 1.0f / pBackBufferSurfaceDesc->Width;
    _invSceneSize.y = 1.0f / pBackBufferSurfaceDesc->Height;

    return S_OK;
}

void FXAAPostProcess::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{
    PostProcess::OnD3D11ReleasingSwapChain(pContentManager);
}