#include "LightBuffer.h"

HRESULT LightBuffer::GSSetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx)
{
	pd3dImmediateContext->GSSetShaderResources(startIdx, 1, &_shaderResourceView);

	return S_OK;
}

HRESULT LightBuffer::VSSetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx)
{
	pd3dImmediateContext->VSSetShaderResources(startIdx, 1, &_shaderResourceView);

	return S_OK;
}

HRESULT LightBuffer::PSSetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx)
{
	pd3dImmediateContext->PSSetShaderResources(startIdx, 1, &_shaderResourceView);

	return S_OK;
}

HRESULT LightBuffer::GSUnsetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx)
{
	ID3D11ShaderResourceView* ppSRVNULL[4] = { NULL };
	pd3dImmediateContext->GSSetShaderResources(startIdx, 4, ppSRVNULL);

	return S_OK;
}

HRESULT LightBuffer::VSUnsetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx)
{
	ID3D11ShaderResourceView* ppSRVNULL[4] = { NULL };
	pd3dImmediateContext->VSSetShaderResources(startIdx, 4, ppSRVNULL);

	return S_OK;
}

HRESULT LightBuffer::PSUnsetShaderResources(ID3D11DeviceContext* pd3dImmediateContext, int startIdx)
{
	ID3D11ShaderResourceView* ppSRVNULL[4] = { NULL };
	pd3dImmediateContext->PSSetShaderResources(startIdx, 4, ppSRVNULL);

	return S_OK;
}

HRESULT LightBuffer::Clear(ID3D11DeviceContext* pd3dImmediateContext)
{
	const float rtclear[] = { _ambientColor.x, _ambientColor.y, _ambientColor.z, 0.0f };
	pd3dImmediateContext->ClearRenderTargetView(_renderTargetView, rtclear);

	return S_OK;
}

HRESULT LightBuffer::SetRenderTargetsAndDepthStencil(ID3D11DeviceContext* pd3dImmediateContext)
{
	pd3dImmediateContext->OMSetRenderTargets(1, &_renderTargetView, NULL);

	return S_OK;
}

HRESULT LightBuffer::SetRenderTargets(ID3D11DeviceContext* pd3dImmediateContext, ID3D11DepthStencilView* dsv)
{
	pd3dImmediateContext->OMSetRenderTargets(1, &_renderTargetView, dsv);

	return S_OK;
}

HRESULT LightBuffer::UnsetRenderTargetsAndDepthStencil(ID3D11DeviceContext* pd3dImmediateContext)
{
	ID3D11RenderTargetView* ppRTVNULL[1] = { NULL };

	pd3dImmediateContext->OMSetRenderTargets(1, ppRTVNULL, NULL);

	return S_OK;
}

HRESULT LightBuffer::UnsetRenderTargets(ID3D11DeviceContext* pd3dImmediateContext, ID3D11DepthStencilView* dsv)
{
	ID3D11RenderTargetView* ppRTVNULL[1] = { NULL };

	pd3dImmediateContext->OMSetRenderTargets(1, ppRTVNULL, dsv);

	return S_OK;
}

HRESULT LightBuffer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}
void LightBuffer::OnD3D11DestroyDevice()
{
}

HRESULT LightBuffer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	// Create the texture
	D3D11_TEXTURE2D_DESC textureDesc = 
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

	V_RETURN(pd3dDevice->CreateTexture2D(&textureDesc, NULL, &_texture));

	// Create the shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = 
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	srvDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_texture, &srvDesc, &_shaderResourceView));

	// Create the render target
	D3D11_RENDER_TARGET_VIEW_DESC rtDesc = 
	{
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_texture, &rtDesc, &_renderTargetView));

	return S_OK;
}
void LightBuffer::OnD3D11ReleasingSwapChain()
{	
	SAFE_RELEASE(_texture);
	SAFE_RELEASE(_shaderResourceView);
	SAFE_RELEASE(_renderTargetView);
}