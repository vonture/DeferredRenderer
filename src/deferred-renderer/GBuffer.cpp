#include "PCH.h"
#include "GBuffer.h"

GBuffer::GBuffer()
{
    for (int i = 0; i < 4; i++)
    {
        _srvs[i] = NULL;
    }

    for (int i = 0; i < 3; i++)
    {
        _rtvs[i] = NULL;
    }

    _dsv = NULL;
    _rodsv = NULL;
}

HRESULT GBuffer::Clear(ID3D11DeviceContext* pd3dImmediateContext)
{
    const float rt0clear[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // Initialize normals to 1 to prevent normalizing the zero vector
    const float rt1clear[] = { 0.0f, 0.0f, -1.0f, 0.0f };
    const float rt2clear[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    pd3dImmediateContext->ClearRenderTargetView(_rtvs[0], rt0clear);
    pd3dImmediateContext->ClearRenderTargetView(_rtvs[1], rt1clear);
    pd3dImmediateContext->ClearRenderTargetView(_rtvs[2], rt2clear);
    pd3dImmediateContext->ClearDepthStencilView(_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    return S_OK;
}

HRESULT GBuffer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    return S_OK;
}

void GBuffer::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
}

HRESULT GBuffer::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    HRESULT hr;

    // Create the textures
    D3D11_TEXTURE2D_DESC rt012Desc =
    {
        pBackBufferSurfaceDesc->Width,//UINT Width;
        pBackBufferSurfaceDesc->Height,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R16G16B16A16_FLOAT,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;
    };

    D3D11_TEXTURE2D_DESC rt3Desc =
    {
        pBackBufferSurfaceDesc->Width,//UINT Width;
        pBackBufferSurfaceDesc->Height,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R24G8_TYPELESS,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;
    };

    ID3D11Texture2D* textures[4];
    V_RETURN(pd3dDevice->CreateTexture2D(&rt012Desc, NULL, &textures[0]));
    V_RETURN(pd3dDevice->CreateTexture2D(&rt012Desc, NULL, &textures[1]));
    V_RETURN(pd3dDevice->CreateTexture2D(&rt012Desc, NULL, &textures[2]));
    V_RETURN(pd3dDevice->CreateTexture2D(&rt3Desc,   NULL, &textures[3]));

    V_RETURN(SetDXDebugName(textures[0], "GBuffer RT0 Texture"));
    V_RETURN(SetDXDebugName(textures[1], "GBuffer RT1 Texture"));
    V_RETURN(SetDXDebugName(textures[2], "GBuffer RT2 Texture"));
    V_RETURN(SetDXDebugName(textures[3], "GBuffer RT3 Texture"));

    // Create the shader resource views
    D3D11_SHADER_RESOURCE_VIEW_DESC rt012rvd =
    {
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
    rt012rvd.Texture2D.MipLevels = 1;

    D3D11_SHADER_RESOURCE_VIEW_DESC rt3rvd =
    {
        DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
    rt3rvd.Texture2D.MipLevels = 1;

    V_RETURN(pd3dDevice->CreateShaderResourceView(textures[0], &rt012rvd, &_srvs[0]));
    V_RETURN(pd3dDevice->CreateShaderResourceView(textures[1], &rt012rvd, &_srvs[1]));
    V_RETURN(pd3dDevice->CreateShaderResourceView(textures[2], &rt012rvd, &_srvs[2]));
    V_RETURN(pd3dDevice->CreateShaderResourceView(textures[3], &rt3rvd,   &_srvs[3]));

    V_RETURN(SetDXDebugName(_srvs[0], "GBuffer RT0 SRV"));
    V_RETURN(SetDXDebugName(_srvs[1], "GBuffer RT1 SRV"));
    V_RETURN(SetDXDebugName(_srvs[2], "GBuffer RT2 SRV"));
    V_RETURN(SetDXDebugName(_srvs[3], "GBuffer RT3 SRV"));

    // Create the render targets
    D3D11_RENDER_TARGET_VIEW_DESC rt012rtvd =
    {
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

    V_RETURN(pd3dDevice->CreateRenderTargetView(textures[0], &rt012rtvd, &_rtvs[0]));
    V_RETURN(pd3dDevice->CreateRenderTargetView(textures[1], &rt012rtvd, &_rtvs[1]));
    V_RETURN(pd3dDevice->CreateRenderTargetView(textures[2], &rt012rtvd, &_rtvs[2]));

    V_RETURN(SetDXDebugName(_rtvs[0], "GBuffer RT0 RTV"));
    V_RETURN(SetDXDebugName(_rtvs[1], "GBuffer RT1 RTV"));
    V_RETURN(SetDXDebugName(_rtvs[2], "GBuffer RT2 RTV"));

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd =
    {
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        D3D11_DSV_DIMENSION_TEXTURE2D,
        0,
    };

    V_RETURN(pd3dDevice->CreateDepthStencilView(textures[3], &dsvd, &_dsv));

    V_RETURN(SetDXDebugName(_dsv, "GBuffer RT3 DSV"));

    // Create the readonly depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC rodsvd =
    {
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        D3D11_DSV_DIMENSION_TEXTURE2D,
        D3D11_DSV_READ_ONLY_DEPTH,
    };

    V_RETURN(pd3dDevice->CreateDepthStencilView(textures[3], &rodsvd, &_rodsv));

    V_RETURN(SetDXDebugName(_rodsv, "GBuffer RT3 Read Only DSV"));

    for(UINT i = 0; i < 4; i++)
    {
        SAFE_RELEASE(textures[i]);
    }

    return S_OK;
}

void GBuffer::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{
    for (int i = 0; i < 4; i++)
    {
        SAFE_RELEASE(_srvs[i]);
    }

    for (int i = 0; i < 3; i++)
    {
        SAFE_RELEASE(_rtvs[i]);
    }
    SAFE_RELEASE(_dsv);
    SAFE_RELEASE(_rodsv);
}