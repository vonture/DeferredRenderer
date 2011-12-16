#include "PCH.h"
#include "LightBuffer.h"

LightBuffer::LightBuffer()
	: _tex(NULL), _srv(NULL), _rtv(NULL), _ambientColor(0.0f, 0.0f, 0.0f),
	  _ambientBrightness(0)
{
}

HRESULT LightBuffer::Clear(ID3D11DeviceContext* pd3dImmediateContext)
{
	const float rtclear[] =
	{ 
		_ambientColor.x * _ambientBrightness,
		_ambientColor.y * _ambientBrightness,
		_ambientColor.z * _ambientBrightness, 
		0.0f 
	};
	pd3dImmediateContext->ClearRenderTargetView(_rtv, rtclear);

	return S_OK;
}


HRESULT LightBuffer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void LightBuffer::OnD3D11DestroyDevice()
{
}

HRESULT LightBuffer::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
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
        DXGI_FORMAT_R16G16B16A16_FLOAT,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;    
    };

	V_RETURN(pd3dDevice->CreateTexture2D(&textureDesc, NULL, &_tex));
	V_RETURN(SetDXDebugName(_tex, "Light Buffer Texture"));

	// Create the shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = 
    {
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	srvDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_tex, &srvDesc, &_srv));
	V_RETURN(SetDXDebugName(_srv, "Light Buffer SRV"));

	// Create the render target
	D3D11_RENDER_TARGET_VIEW_DESC rtDesc = 
	{
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_tex, &rtDesc, &_rtv));
	V_RETURN(SetDXDebugName(_rtv, "Light Buffer RTV"));

	return S_OK;
}
void LightBuffer::OnD3D11ReleasingSwapChain()
{	
	SAFE_RELEASE(_tex);
	SAFE_RELEASE(_srv);
	SAFE_RELEASE(_rtv);
}