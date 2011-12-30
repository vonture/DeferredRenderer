#include "PCH.h"
#include "LightBuffer.h"

LightBuffer::LightBuffer()
	: _ambientColor(0.0f, 0.0f, 0.0f), _ambientBrightness(0)
{
	for (UINT i = 0; i < 2; i++)
	{
		_tex[i] = NULL;
		_srv[i] = NULL;
		_rtv[i] = NULL;
	}
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
	pd3dImmediateContext->ClearRenderTargetView(_rtv[0], rtclear);
	pd3dImmediateContext->ClearRenderTargetView(_rtv[1], rtclear);

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

	V_RETURN(pd3dDevice->CreateTexture2D(&textureDesc, NULL, &_tex[0]));
	V_RETURN(pd3dDevice->CreateTexture2D(&textureDesc, NULL, &_tex[1]));

	V_RETURN(SetDXDebugName(_tex[0], "Light Buffer Geometry Texture"));
	V_RETURN(SetDXDebugName(_tex[1], "Light Buffer Particle Texture"));

	// Create the shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = 
    {
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	srvDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_tex[0], &srvDesc, &_srv[0]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_tex[1], &srvDesc, &_srv[1]));

	V_RETURN(SetDXDebugName(_srv[0], "Light Buffer Geometry SRV"));
	V_RETURN(SetDXDebugName(_srv[1], "Light Buffer Particle SRV"));

	// Create the render target
	D3D11_RENDER_TARGET_VIEW_DESC rtDesc = 
	{
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_tex[0], &rtDesc, &_rtv[0]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_tex[1], &rtDesc, &_rtv[1]));
	
	V_RETURN(SetDXDebugName(_rtv[0], "Light Buffer Geometry RTV"));
	V_RETURN(SetDXDebugName(_rtv[1], "Light Buffer Particle RTV"));

	return S_OK;
}
void LightBuffer::OnD3D11ReleasingSwapChain()
{	
	for (UINT i = 0; i < 2; i++)
	{
		SAFE_RELEASE(_tex[i]);
		SAFE_RELEASE(_srv[i]);
		SAFE_RELEASE(_rtv[i]);
	}
}