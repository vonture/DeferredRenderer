#include "PCH.h"
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
	const float rt0clear[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Initialize normals to 1 to prevent normalizing the zero vector
	const float rt1clear[] = { 1.0f, 1.0f, 1.0f, 0.0f }; 
	const float rt2clear[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	
	pd3dImmediateContext->ClearRenderTargetView(_renderTargetViews[0], rt0clear);
	pd3dImmediateContext->ClearRenderTargetView(_renderTargetViews[1], rt1clear);
	pd3dImmediateContext->ClearRenderTargetView(_renderTargetViews[2], rt2clear);
	pd3dImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	return S_OK;
}

HRESULT GBuffer::SetRenderTargetsAndDepthStencil(ID3D11DeviceContext* pd3dImmediateContext)
{
	pd3dImmediateContext->OMSetRenderTargets(3, &_renderTargetViews[0], _depthStencilView);

	return S_OK;
}

HRESULT GBuffer::SetRenderTargets(ID3D11DeviceContext* pd3dImmediateContext, ID3D11DepthStencilView* dsv)
{
	pd3dImmediateContext->OMSetRenderTargets(3, &_renderTargetViews[0], dsv);

	return S_OK;
}

HRESULT GBuffer::UnsetRenderTargetsAndDepthStencil(ID3D11DeviceContext* pd3dImmediateContext)
{
	ID3D11RenderTargetView* ppRTVNULL[3] = { NULL, NULL, NULL };

	pd3dImmediateContext->OMSetRenderTargets(3, ppRTVNULL, NULL);

	return S_OK;
}

HRESULT GBuffer::UnsetRenderTargets(ID3D11DeviceContext* pd3dImmediateContext, ID3D11DepthStencilView* dsv)
{
	ID3D11RenderTargetView* ppRTVNULL[3] = { NULL, NULL, NULL };

	pd3dImmediateContext->OMSetRenderTargets(3, ppRTVNULL, dsv);

	return S_OK;
}

HRESULT GBuffer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void GBuffer::OnD3D11DestroyDevice()
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

	V_RETURN(pd3dDevice->CreateTexture2D(&rt012Desc, NULL, &_textures[0]));
	V_RETURN(pd3dDevice->CreateTexture2D(&rt012Desc, NULL, &_textures[1]));
	V_RETURN(pd3dDevice->CreateTexture2D(&rt012Desc, NULL, &_textures[2]));
	V_RETURN(pd3dDevice->CreateTexture2D(&rt3Desc,   NULL, &_textures[3]));

	V_RETURN(SetDXDebugName(_textures[0], "RT0 Texture"));
	V_RETURN(SetDXDebugName(_textures[1], "RT1 Texture"));
	V_RETURN(SetDXDebugName(_textures[2], "RT2 Texture"));
	V_RETURN(SetDXDebugName(_textures[3], "RT3 Texture"));

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

	V_RETURN(pd3dDevice->CreateShaderResourceView(_textures[0], &rt012rvd, &_shaderResourceViews[0]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_textures[1], &rt012rvd, &_shaderResourceViews[1]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_textures[2], &rt012rvd, &_shaderResourceViews[2]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_textures[3], &rt3rvd,   &_shaderResourceViews[3]));

	V_RETURN(SetDXDebugName(_shaderResourceViews[0], "RT0 SRV"));
	V_RETURN(SetDXDebugName(_shaderResourceViews[1], "RT1 SRV"));
	V_RETURN(SetDXDebugName(_shaderResourceViews[2], "RT2 SRV"));
	V_RETURN(SetDXDebugName(_shaderResourceViews[3], "RT3 SRV"));

	// Create the render targets
	D3D11_RENDER_TARGET_VIEW_DESC rt012rtvd = 
	{
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_textures[0], &rt012rtvd, &_renderTargetViews[0]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_textures[1], &rt012rtvd, &_renderTargetViews[1]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_textures[2], &rt012rtvd, &_renderTargetViews[2]));

	V_RETURN(SetDXDebugName(_renderTargetViews[0], "RT0 RTV"));
	V_RETURN(SetDXDebugName(_renderTargetViews[1], "RT1 RTV"));
	V_RETURN(SetDXDebugName(_renderTargetViews[2], "RT2 RTV"));

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd =
	{
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		0,
	};

	V_RETURN(pd3dDevice->CreateDepthStencilView(_textures[3], &dsvd, &_depthStencilView));
	
	V_RETURN(SetDXDebugName(_renderTargetViews[3], "RT3 DSV"));

	// Create the readonly depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC rodsvd =
	{
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		D3D11_DSV_READ_ONLY_DEPTH,
	};

	V_RETURN(pd3dDevice->CreateDepthStencilView(_textures[3], &rodsvd, &_readonlyDepthStencilView));
	
	V_RETURN(SetDXDebugName(_renderTargetViews[3], "RT3 Read Only DSV"));
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
	SAFE_RELEASE(_depthStencilView);
	SAFE_RELEASE(_readonlyDepthStencilView);
}