#include "SkyPostProcess.h"

SkyPostProcess::SkyPostProcess()
	: _skyColor(0.0f, 0.0f, 0.0f), _sunColor(0.0f, 0.0f, 0.0f), _sunDirection(0.0f, 0.0f, 0.0f),
	  _sunWidth(0.0f), _enableSun(false), _skyPS(NULL), _skyProperties(NULL)
{
}

SkyPostProcess::~SkyPostProcess()
{
}

void SkyPostProcess::SetSunDirection(const XMFLOAT3& sunDir)
{
	XMVECTOR dir = XMLoadFloat3(&sunDir);
	XMStoreFloat3(&_sunDirection, XMVector3Normalize(dir));
}


HRESULT SkyPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
	ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer)
{
	DXUT_BeginPerfEvent(D3DCOLOR_COLORVALUE(0.0f, 0.0f, 1.0f, 1.0f), L"Sky");

	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;	

	// Render the sky by using the gbuffer's depth stencil to render the sky in the locations without
	// depth values

	// Prepare all the settings and map them
	XMFLOAT4X4 fViewProj = camera->GetViewProjection();
	XMMATRIX viewProj = XMLoadFloat4x4(&fViewProj);

	XMVECTOR det;	
	XMMATRIX invViewProj = XMMatrixInverse(&det, viewProj);

	V_RETURN(pd3dImmediateContext->Map(_skyProperties, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_SKY_PROPERTIES* skyProperties = (CB_SKY_PROPERTIES*)mappedResource.pData;
		
	skyProperties->SkyColor = _skyColor;
	skyProperties->SunColor = _sunColor;
	skyProperties->SunDirection = _sunDirection;
	skyProperties->SunWidth = _sunWidth;
	skyProperties->SunEnabled = _enableSun ? 1 : 0;
	skyProperties->CameraPosition = camera->GetPosition();
	XMStoreFloat4x4(&skyProperties->InverseViewProjection, XMMatrixTranspose(invViewProj));

	pd3dImmediateContext->Unmap(_skyProperties, 0);

	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_skyProperties);

	// Set the render targets
	pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, NULL);

	// Prepare the blend, depth and sampler
	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

	Quad* fsQuad = GetFullScreenQuad();

	ID3D11SamplerState* sampler = GetSamplerStates()->GetPoint();
	pd3dImmediateContext->PSSetSamplers(0, 1, &sampler);

	// Prepare the SRVs
	ID3D11ShaderResourceView* ppSRVs[2] = { src, gBuffer->GetShaderResourceView(3) };
	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVs);

	// Render
	V_RETURN(fsQuad->Render(pd3dImmediateContext, _skyPS));

	// Null the SRVs
	ID3D11ShaderResourceView* ppSRVNULL[2] = { NULL, NULL };
	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVNULL);

	DXUT_EndPerfEvent();

	return S_OK;
}

HRESULT SkyPostProcess:: OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	// Load the shaders
	ID3DBlob* pBlob = NULL;

	V_RETURN( CompileShaderFromFile( L"Sky.hlsl", "PS_Sky", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_skyPS));
	SAFE_RELEASE(pBlob);

	// Create the buffer
	D3D11_BUFFER_DESC bufferDesc =
	{
		sizeof(CB_SKY_PROPERTIES), //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_skyProperties));

	return S_OK;
}

void SkyPostProcess::OnD3D11DestroyDevice()
{
	PostProcess::OnD3D11DestroyDevice();

	SAFE_RELEASE(_skyPS);
	SAFE_RELEASE(_skyProperties);
}

HRESULT SkyPostProcess::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void SkyPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();
}