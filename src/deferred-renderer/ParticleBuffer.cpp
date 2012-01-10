#include "PCH.h"
#include "ParticleBuffer.h"

ParticleBuffer::ParticleBuffer()
{
	for (int i = 0; i < 3; i++)
	{
		_textures[i] = NULL;
		_srvs[i] = NULL;
		_rtvs[i] = NULL;
	}
}

HRESULT ParticleBuffer::Clear(ID3D11DeviceContext* pd3dImmediateContext)
{
	const float rt0clear[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	const float rt1clear[] = { 0.0f, 0.0f, 0.0f, 0.0f }; 
	const float rt2clear[] = { 0.0f, 0.0f, 0.0f, 0.0f }; 

	pd3dImmediateContext->ClearRenderTargetView(_rtvs[0], rt0clear);
	pd3dImmediateContext->ClearRenderTargetView(_rtvs[1], rt1clear);
	pd3dImmediateContext->ClearRenderTargetView(_rtvs[2], rt2clear);

	return S_OK;
}

HRESULT ParticleBuffer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void ParticleBuffer::OnD3D11DestroyDevice()
{
}

HRESULT ParticleBuffer::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
	IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	// Create the textures
	D3D11_TEXTURE2D_DESC texDesc = 
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

	V_RETURN(pd3dDevice->CreateTexture2D(&texDesc, NULL, &_textures[0]));
	V_RETURN(pd3dDevice->CreateTexture2D(&texDesc, NULL, &_textures[1]));
	V_RETURN(pd3dDevice->CreateTexture2D(&texDesc, NULL, &_textures[2]));

	V_RETURN(SetDXDebugName(_textures[0], "Particle Buffer RT0 Texture"));
	V_RETURN(SetDXDebugName(_textures[1], "Particle Buffer RT1 Texture"));
	V_RETURN(SetDXDebugName(_textures[2], "Particle Buffer RT2 Texture"));

	// Create the shader resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = 
	{
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};
	SRVDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_textures[0], &SRVDesc, &_srvs[0]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_textures[1], &SRVDesc, &_srvs[1]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_textures[2], &SRVDesc, &_srvs[2]));

	V_RETURN(SetDXDebugName(_srvs[0], "Particle Buffer RT0 SRV"));
	V_RETURN(SetDXDebugName(_srvs[1], "Particle Buffer RT1 SRV"));
	V_RETURN(SetDXDebugName(_srvs[2], "Particle Buffer RT2 SRV"));

	// Create the render targets
	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = 
	{
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		D3D11_RTV_DIMENSION_TEXTURE2D,
		0,
		0
	};

	V_RETURN(pd3dDevice->CreateRenderTargetView(_textures[0], &RTVDesc, &_rtvs[0]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_textures[1], &RTVDesc, &_rtvs[1]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_textures[2], &RTVDesc, &_rtvs[2]));

	V_RETURN(SetDXDebugName(_rtvs[0], "Particle Buffer RT0 RTV"));
	V_RETURN(SetDXDebugName(_rtvs[1], "Particle Buffer RT1 RTV"));
	V_RETURN(SetDXDebugName(_rtvs[2], "Particle Buffer RT2 RTV"));

	return S_OK;
}

void ParticleBuffer::OnD3D11ReleasingSwapChain()
{
	for (int i = 0; i < 3; i++)
	{
		SAFE_RELEASE(_textures[i]);
		SAFE_RELEASE(_srvs[i]);
		SAFE_RELEASE(_rtvs[i]);
	}
}
