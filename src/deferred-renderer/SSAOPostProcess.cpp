#include "PCH.h"
#include "SSAOPostProcess.h"
#include "Logger.h"

const UINT SSAOPostProcess::SSAO_SAMPLE_COUNTS[NUM_SSAO_SAMPLE_COUNTS] =
{
#ifdef ALL_PRESETS
    SSAO_SAMPLE_COUNT_MAX / 64,
    SSAO_SAMPLE_COUNT_MAX / 32,
    SSAO_SAMPLE_COUNT_MAX / 16,
    SSAO_SAMPLE_COUNT_MAX / 8,
    SSAO_SAMPLE_COUNT_MAX / 4,
    SSAO_SAMPLE_COUNT_MAX / 2,
    SSAO_SAMPLE_COUNT_MAX / 1
#else
    SSAO_SAMPLE_COUNT_MAX / 2,
    SSAO_SAMPLE_COUNT_MAX / 1
#endif
};

SSAOPostProcess::SSAOPostProcess()
    : _aoRTV(NULL), _aoSRV(NULL), _blurTempRTV(NULL), _blurTempSRV(NULL), _aoPropertiesBuffer(NULL),
    _randomSRV(NULL)
{
    SetIsAdditive(false);

    for (UINT i = 0; i < NUM_SSAO_SAMPLE_COUNTS; i++)
    {
        _aoPSs[i] = NULL;
        _sampleDirectionsBuffers[i] = NULL;
    }

    for (UINT i = 0; i < 2; i++)
    {
        _hBlurPS[i] = NULL;
        _vBlurPS[i] = NULL;
        _compositePS[i] = NULL;
    }

    // Initialize some parameters to default values
    SetSampleRadius(30.0f);
    SetBlurSigma(1.8f);
    SetDepthTreshold(4.0f);
    SetHalfResolution(false);

#ifdef ALL_PRESETS
    SetSampleCountIndex(3);
#else
    SetSampleCountIndex(1);
#endif
}

HRESULT SSAOPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
                                ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, ParticleBuffer* pBuffer,LightBuffer* lightBuffer)
{
    BEGIN_EVENT_D3D(L"SSAO");

    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    // Save the old viewport
    D3D11_VIEWPORT vpOld[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
    UINT nViewPorts = 1;
    pd3dImmediateContext->RSGetViewports(&nViewPorts, vpOld);

    // Prepare all the settings and map them
    V_RETURN(pd3dImmediateContext->Map(_aoPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    CB_AO_PROPERTIES* aoProperties = (CB_AO_PROPERTIES*)mappedResource.pData;

    XMFLOAT4X4 fViewProj = camera->GetViewProjection();
    XMMATRIX viewProj = XMLoadFloat4x4(&fViewProj);

    XMVECTOR det;
    XMMATRIX invViewProj = XMMatrixInverse(&det, viewProj);

    XMStoreFloat4x4(&aoProperties->ViewProjection, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&aoProperties->InverseViewProjection, XMMatrixTranspose(invViewProj));
    aoProperties->SampleRadius = _sampleRadius;
    aoProperties->BlurSigma = _blurSigma;
    aoProperties->GaussianNumerator = 1.0f / sqrt(2.0f * Pi * _blurSigma * _blurSigma);
    aoProperties->CameraNearClip = camera->GetNearClip();
    aoProperties->CameraFarClip = camera->GetFarClip();
    aoProperties->DepthThreshold = _depthThreshold;
    aoProperties->InverseSceneSize = _invSceneSize;

    pd3dImmediateContext->Unmap(_aoPropertiesBuffer, 0);

    // Set the constant buffers
    ID3D11Buffer* cbs[2] =
    {
        _aoPropertiesBuffer,
        _sampleDirectionsBuffers[_sampleCountIndex],
    };

    pd3dImmediateContext->PSSetConstantBuffers(0, 2, cbs);

    // Set all the device states
    ID3D11SamplerState* samplers[2] =
    {
        GetSamplerStates()->GetPointClamp(),
        GetSamplerStates()->GetLinearClamp(),
    };

    pd3dImmediateContext->PSSetSamplers(0, 2, samplers);

    pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);

    float blendFactor[4] = {1, 1, 1, 1};
    pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

    Quad* fsQuad = GetFullScreenQuad();

    float resolutionMult = _halfRes ? 0.5f : 1.0f;

    D3D11_VIEWPORT vp;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.Width = vpOld[0].Width * resolutionMult;
    vp.Height = vpOld[0].Height * resolutionMult;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;

    pd3dImmediateContext->RSSetViewports(1, &vp);

    // Render the SSAO
    BEGIN_EVENT_D3D(L"Occlusion");

    pd3dImmediateContext->OMSetRenderTargets(1, &_aoRTV, NULL);

    ID3D11ShaderResourceView* ppSRVAO[2] =
    {
        gBuffer->GetDepthSRV(),
        _randomSRV
    };
    pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVAO);

    V_RETURN(fsQuad->Render(pd3dImmediateContext, _aoPSs[_sampleCountIndex]->PixelShader));

    END_EVENT_D3D(L"");

    // Blur AO texture using normals for edge detection
    ID3D11ShaderResourceView* pphBlurSRVs[2] = { _aoSRV, gBuffer->GetNormalSRV() };

    BEGIN_EVENT_D3D(L"Blur horizontal");
    pd3dImmediateContext->OMSetRenderTargets(1, &_blurTempRTV, NULL);
    pd3dImmediateContext->PSSetShaderResources(0, 2, pphBlurSRVs);
    V_RETURN(fsQuad->Render(pd3dImmediateContext, _hBlurPS[_halfRes ? 1 : 0]->PixelShader));
    END_EVENT_D3D(L"");

    ID3D11ShaderResourceView* ppSRVNULL1[1] = { NULL };
    pd3dImmediateContext->PSSetShaderResources(0, 1, ppSRVNULL1);

    BEGIN_EVENT_D3D(L"Blur vertical");
    pd3dImmediateContext->OMSetRenderTargets(1, &_aoRTV, NULL);
    pd3dImmediateContext->PSSetShaderResources(0, 1, &_blurTempSRV);
    V_RETURN(fsQuad->Render(pd3dImmediateContext, _vBlurPS[_halfRes ? 1 : 0]->PixelShader));
    END_EVENT_D3D(L"");

    // Re-apply the old viewport
    pd3dImmediateContext->RSSetViewports(nViewPorts, vpOld);

    // Composite
    BEGIN_EVENT_D3D(L"Composite");

    pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, NULL);

    ID3D11ShaderResourceView* ppAOCompositeSRV[2] = { src, _aoSRV };
    pd3dImmediateContext->PSSetShaderResources(0, 2, ppAOCompositeSRV);

    V_RETURN(fsQuad->Render(pd3dImmediateContext, _compositePS[_halfRes ? 1 : 0]->PixelShader));

    END_EVENT_D3D(L"");

    // Unset the SRVs
    ID3D11ShaderResourceView* const ppSRVNULL2[2] = { NULL, NULL };
    pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVNULL2);

    END_EVENT_D3D(L"");

    return S_OK;
}

HRESULT SSAOPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
                                             const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

    char debugName[512];
    D3D_SHADER_MACRO sampleCountMacros[] =
    {
        { "SSAO_SAMPLE_COUNT", "" },
        { "RAND_TEX_SIZE", "" },
        NULL,
    };

    // Load the shaders
    PixelShaderOptions aoPSOpts =
    {
        "PS_SSAO", // const char* EntryPoint;
        sampleCountMacros, // D3D_SHADER_MACRO* Defines;
        debugName // const char* DebugName;
    };

    char randTexSizeString[6];
    sprintf_s(randTexSizeString, "%u", RANDOM_TEXTURE_SIZE);
    sampleCountMacros[1].Definition = randTexSizeString;

    char sampleCountString[6];
    for (UINT i = 0; i < NUM_SSAO_SAMPLE_COUNTS; i++)
    {
        sprintf_s(sampleCountString, "%i", SSAO_SAMPLE_COUNTS[i]);
        sampleCountMacros[0].Definition = sampleCountString;

        sprintf_s(debugName, "SSAO AO (sample count = %s) PS", sampleCountString);

        V_RETURN(pContentManager->LoadContent(pd3dDevice, L"SSAO.hlsl", &aoPSOpts, &_aoPSs[i]));
    }

    D3D_SHADER_MACRO resMacros[] =
    {
        { "SSAO_HALF_RES", "" },
        NULL,
    };

    // Load the shaders
    PixelShaderOptions otherPSOpts =
    {
        "", // const char* EntryPoint;
        resMacros, // D3D_SHADER_MACRO* Defines;
        debugName // const char* DebugName;
    };

    for (UINT i = 0; i < 2; i++)
    {
        resMacros[0].Definition = (i == 0) ? "0" : "1";

        // hblur
        sprintf_s(debugName, "SSAO horizontal blur (half resolution = %u)", i);
        otherPSOpts.EntryPoint = "PS_BlurHorizontal";
        V_RETURN(pContentManager->LoadContent(pd3dDevice, L"SSAO.hlsl", &otherPSOpts, &_hBlurPS[i]));

        // vblur
        sprintf_s(debugName, "SSAO vertical blur (half resolution = %u)", i);
        otherPSOpts.EntryPoint = "PS_BlurVertical";
        V_RETURN(pContentManager->LoadContent(pd3dDevice, L"SSAO.hlsl", &otherPSOpts, &_vBlurPS[i]));

        // composite
        sprintf_s(debugName, "SSAO composite  (half resolution = %u)", i);
        otherPSOpts.EntryPoint = "PS_SSAO_Composite";
        V_RETURN(pContentManager->LoadContent(pd3dDevice, L"SSAO.hlsl", &otherPSOpts, &_compositePS[i]));
    }

    // Create the buffers
    D3D11_BUFFER_DESC bufferDesc =
    {
        0, //UINT ByteWidth;
        D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
        D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
        D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
        0, //UINT MiscFlags;
        0, //UINT StructureByteStride;
    };

    bufferDesc.ByteWidth = sizeof(CB_AO_PROPERTIES);
    V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_aoPropertiesBuffer));
    V_RETURN(SetDXDebugName(_aoPropertiesBuffer, "SSAO properties buffer"));

    // Create the random sample directions
    XMFLOAT4 sampleDirections[SSAO_SAMPLE_COUNT_MAX];
    for (UINT i = 0; i < SSAO_SAMPLE_COUNT_MAX; i++)
    {
        float randAngle = ((rand() % 1000) / 1000.0f) * 2.0f * Pi;
        float randLen = ((rand() % 1000) / 1000.0f);

        sampleDirections[i] = XMFLOAT4(randAngle, randLen, 0.0f, 0.0f);
    }

    D3D11_SUBRESOURCE_DATA directionInitData;
    directionInitData.pSysMem = &sampleDirections;
    directionInitData.SysMemPitch = 0;
    directionInitData.SysMemSlicePitch = 0;

    char bufferDebugName[256];
    for (UINT i = 0; i < NUM_SSAO_SAMPLE_COUNTS; i++)
    {
        bufferDesc.ByteWidth = SSAO_SAMPLE_COUNTS[i] * sizeof(XMFLOAT4);

        V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, &directionInitData, &_sampleDirectionsBuffers[i]));

        sprintf_s(bufferDebugName, "SSAO sample directions buffer (samples = %u)", SSAO_SAMPLE_COUNTS[i]);
        V_RETURN(SetDXDebugName(_sampleDirectionsBuffers[i], bufferDebugName));
    }

    // The random texture and srv are not dependent on the back buffer, create them here
    D3D11_TEXTURE2D_DESC randomTextureDesc =
    {
        RANDOM_TEXTURE_SIZE,//UINT Width;
        RANDOM_TEXTURE_SIZE,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R32_FLOAT,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;
    };

    float randomData[RANDOM_TEXTURE_SIZE * RANDOM_TEXTURE_SIZE];
    for (UINT i = 0; i < RANDOM_TEXTURE_SIZE * RANDOM_TEXTURE_SIZE; i++)
    {
        randomData[i] = ((rand() % 1000) / 1000.0f) * 2.0f * Pi;
    }

    D3D11_SUBRESOURCE_DATA randomInitData;
    randomInitData.pSysMem = &randomData;
    randomInitData.SysMemPitch = RANDOM_TEXTURE_SIZE * sizeof(float);
    randomInitData.SysMemSlicePitch = 0;

    ID3D11Texture2D* randomTexture;
    V_RETURN(pd3dDevice->CreateTexture2D(&randomTextureDesc, &randomInitData, &randomTexture));

    // Create the shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC randomSRVDesc =
    {
        DXGI_FORMAT_R32_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
    randomSRVDesc.Texture2D.MipLevels = 1;

    V_RETURN(pd3dDevice->CreateShaderResourceView(randomTexture, &randomSRVDesc, &_randomSRV));
    V_RETURN(SetDXDebugName(_randomSRV, "SSAO random SRV"));

    SAFE_RELEASE(randomTexture);

    _invSceneSize.x = 1.0f / pBackBufferSurfaceDesc->Width;
    _invSceneSize.y = 1.0f / pBackBufferSurfaceDesc->Height;

    return S_OK;
}

void SSAOPostProcess::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
    PostProcess::OnD3D11DestroyDevice(pContentManager);

    SAFE_RELEASE(_randomSRV);

    for (UINT i = 0; i < NUM_SSAO_SAMPLE_COUNTS; i++)
    {
        SAFE_CM_RELEASE(pContentManager, _aoPSs[i]);
        SAFE_RELEASE(_sampleDirectionsBuffers[i]);
    }

    for (UINT i = 0; i < 2; i++)
    {
        SAFE_CM_RELEASE(pContentManager, _hBlurPS[i]);
        SAFE_CM_RELEASE(pContentManager, _vBlurPS[i]);
        SAFE_CM_RELEASE(pContentManager, _compositePS[i]);
    }

    SAFE_RELEASE(_aoPropertiesBuffer);
}

HRESULT SSAOPostProcess::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
                                                 IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

    // Create the AO texture components
    D3D11_TEXTURE2D_DESC aoTextureDesc =
    {
        pBackBufferSurfaceDesc->Width,//UINT Width;
        pBackBufferSurfaceDesc->Height,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R16_FLOAT,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;
    };

    ID3D11Texture2D* aoTexture;
    V_RETURN(pd3dDevice->CreateTexture2D(&aoTextureDesc, NULL, &aoTexture));

    ID3D11Texture2D* blurTempTexture;
    V_RETURN(pd3dDevice->CreateTexture2D(&aoTextureDesc, NULL, &blurTempTexture));

    D3D11_RENDER_TARGET_VIEW_DESC aoRTVDesc =
    {
        DXGI_FORMAT_R16_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

    V_RETURN(pd3dDevice->CreateRenderTargetView(aoTexture, &aoRTVDesc, &_aoRTV));
    V_RETURN(SetDXDebugName(_aoRTV, "SSAO AO RTV"));

    V_RETURN(pd3dDevice->CreateRenderTargetView(blurTempTexture, &aoRTVDesc, &_blurTempRTV));
    V_RETURN(SetDXDebugName(_blurTempRTV, "SSAO blur temp RTV"));

    D3D11_SHADER_RESOURCE_VIEW_DESC aoSRVDesc =
    {
        DXGI_FORMAT_R16_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
    aoSRVDesc.Texture2D.MipLevels = 1;

    V_RETURN(pd3dDevice->CreateShaderResourceView(aoTexture, &aoSRVDesc, &_aoSRV));
    V_RETURN(SetDXDebugName(_aoSRV, "SSAO AO SRV"));

    V_RETURN(pd3dDevice->CreateShaderResourceView(blurTempTexture, &aoSRVDesc, &_blurTempSRV));
    V_RETURN(SetDXDebugName(_blurTempSRV, "SSAO blur temp SRV"));

    SAFE_RELEASE(aoTexture);
    SAFE_RELEASE(blurTempTexture);

    _invSceneSize.x = 1.0f / pBackBufferSurfaceDesc->Width;
    _invSceneSize.y = 1.0f / pBackBufferSurfaceDesc->Height;

    return S_OK;
}

void SSAOPostProcess::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{
    PostProcess::OnD3D11ReleasingSwapChain(pContentManager);

    SAFE_RELEASE(_aoRTV);
    SAFE_RELEASE(_aoSRV);

    SAFE_RELEASE(_blurTempRTV);
    SAFE_RELEASE(_blurTempSRV);
}