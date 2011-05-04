#include "DXUT.h"
#include "GBuffer.h"

HRESULT GBuffer::GSSetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx)
{
	pd3dImmediateContext->GSSetShaderResources(startIdx, 4, &_shaderResourceViews[0]);

	return S_OK;
}

HRESULT GBuffer::VSSetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx)
{
	pd3dImmediateContext->VSSetShaderResources(startIdx, 4, &_shaderResourceViews[0]);

	return S_OK;
}

HRESULT GBuffer::PSSetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx)
{
	pd3dImmediateContext->PSSetShaderResources(startIdx, 4, &_shaderResourceViews[0]);

	return S_OK;
}

HRESULT GBuffer::GSUnsetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx)
{
	ID3D11ShaderResourceView* ppSRVNULL[4] = { NULL, NULL, NULL, NULL };
	pd3dImmediateContext->GSSetShaderResources(startIdx, 4, ppSRVNULL);

	return S_OK;
}

HRESULT GBuffer::VSUnsetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx)
{
	ID3D11ShaderResourceView* ppSRVNULL[4] = { NULL, NULL, NULL, NULL };
	pd3dImmediateContext->VSSetShaderResources(startIdx, 4, ppSRVNULL);

	return S_OK;
}

HRESULT GBuffer::PSUnsetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx)
{
	ID3D11ShaderResourceView* ppSRVNULL[4] = { NULL, NULL, NULL, NULL };
	pd3dImmediateContext->PSSetShaderResources(startIdx, 4, ppSRVNULL);

	return S_OK;
}

HRESULT GBuffer::Clear(ID3D11DeviceContext* pd3dImmediateContext)
{
	const float rt02clear[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	const float rt1clear[] = { 0.5f, 0.5f, 0.5f, 0.0f };

	pd3dImmediateContext->ClearRenderTargetView(_renderTargetViews[0], rt02clear);
	pd3dImmediateContext->ClearRenderTargetView(_renderTargetViews[1], rt1clear);
	pd3dImmediateContext->ClearRenderTargetView(_renderTargetViews[2], rt02clear);
	pd3dImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return S_OK;
}

HRESULT GBuffer::SetRenderTargets(ID3D11DeviceContext* pd3dImmediateContext)
{
	pd3dImmediateContext->OMSetRenderTargets(3, &_renderTargetViews[0], _depthStencilView);

	return S_OK;
}

HRESULT GBuffer::UnsetRenderTargets(ID3D11DeviceContext* pd3dImmediateContext)
{
	ID3D11RenderTargetView* ppRTVNULL[3] = { NULL, NULL, NULL };

	pd3dImmediateContext->OMSetRenderTargets(3, ppRTVNULL, NULL);

	return S_OK;
}

HRESULT GBuffer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void GBuffer::OnD3D11DestroyDevice()
{	
}

HRESULT GBuffer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
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
        DXGI_FORMAT_R32G32B32A32_FLOAT,//DXGI_FORMAT Format;
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
        DXGI_FORMAT_R32_TYPELESS,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;    
    };

	V_RETURN(pd3dDevice->CreateTexture2D(&rt012Desc, NULL, &_textures[0]));
	V_RETURN(pd3dDevice->CreateTexture2D(&rt012Desc, NULL, &_textures[1]));
	V_RETURN(pd3dDevice->CreateTexture2D(&rt012Desc, NULL, &_textures[2]));
	V_RETURN(pd3dDevice->CreateTexture2D(&rt3Desc,   NULL, &_textures[3]));

	DXUT_SetDebugName(_textures[0], "RT0 Texture");
	DXUT_SetDebugName(_textures[1], "RT1 Texture");
	DXUT_SetDebugName(_textures[2], "RT2 Texture");
	DXUT_SetDebugName(_textures[3], "RT3 Texture");

	// Create the shader resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC rt012rvd = 
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	rt012rvd.Texture2D.MipLevels = 1;

	D3D11_SHADER_RESOURCE_VIEW_DESC rt3rvd = 
    {
        DXGI_FORMAT_R32_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	rt3rvd.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_textures[0], &rt012rvd, &_shaderResourceViews[0]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_textures[1], &rt012rvd, &_shaderResourceViews[1]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_textures[2], &rt012rvd, &_shaderResourceViews[2]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_textures[3], &rt3rvd,   &_shaderResourceViews[3]));

	DXUT_SetDebugName(_shaderResourceViews[0], "RT0 SRV");
	DXUT_SetDebugName(_shaderResourceViews[1], "RT1 SRV");
	DXUT_SetDebugName(_shaderResourceViews[2], "RT2 SRV");
	DXUT_SetDebugName(_shaderResourceViews[3], "RT3 SRV");

	// Create the render targets
	D3D11_RENDER_TARGET_VIEW_DESC rt012rtvd = 
	{
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_textures[0], &rt012rtvd, &_renderTargetViews[0]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_textures[1], &rt012rtvd, &_renderTargetViews[1]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_textures[2], &rt012rtvd, &_renderTargetViews[2]));

	DXUT_SetDebugName(_renderTargetViews[0], "RT0 RTV");
	DXUT_SetDebugName(_renderTargetViews[1], "RT1 RTV");
	DXUT_SetDebugName(_renderTargetViews[2], "RT2 RTV");

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd =
	{
		DXGI_FORMAT_D32_FLOAT,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		0,
	};

	V_RETURN(pd3dDevice->CreateDepthStencilView(_textures[3], &dsvd, &_depthStencilView));

	DXUT_SetDebugName(_renderTargetViews[3], "RT3 DSV");

	// Create the depth stencil state
	D3D11_DEPTH_STENCIL_DESC dsd;
    ZeroMemory( &dsd, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
    dsd.DepthEnable = FALSE;
    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsd.DepthFunc = D3D11_COMPARISON_LESS;
    dsd.StencilEnable = FALSE;

    V_RETURN(pd3dDevice->CreateDepthStencilState(&dsd, &_depthStencilState));

	DXUT_SetDebugName(_depthStencilState, "GBuffer Depth Stencil State");

	return S_OK;
}

void GBuffer::OnD3D11ReleasingSwapChain()
{
	for (int i = 0; i < 4; i++)
	{
		SAFE_RELEASE(_textures[i]);
		SAFE_RELEASE(_shaderResourceViews[i]);
	}

	for (int i = 0; i < 3; i++)
	{
		SAFE_RELEASE(_renderTargetViews[i]);
	}

	SAFE_RELEASE(_depthStencilState);
	SAFE_RELEASE(_depthStencilView);
}