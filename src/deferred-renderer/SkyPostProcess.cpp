#include "PCH.h"
#include "SkyPostProcess.h"
#include "Logger.h"

SkyPostProcess::SkyPostProcess()
	: _sunDisabledPS(NULL), _sunEnabledPS(NULL), _skyProperties(NULL)
{
	SetIsAdditive(true);

	SetSunColor(XMFLOAT3(1.0f, 0.8f, 0.5f));
	SetSkyColor(XMFLOAT3(0.2f, 0.5f, 1.0f));
	SetSunDirection(XMFLOAT3(0.4f, 0.9f, 0.2f));
	SetSunWidth(0.05f);
	SetSunEnabled(true);
	SetSunIntensity(4.0f);
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
	BEGIN_EVENT_D3D(L"Sky");

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
	skyProperties->SunIntensity = _sunIntensity;
	skyProperties->CameraPosition = camera->GetPosition();
	XMStoreFloat4x4(&skyProperties->InverseViewProjection, XMMatrixTranspose(invViewProj));

	pd3dImmediateContext->Unmap(_skyProperties, 0);

	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_skyProperties);

	// Set the render targets
	pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, gBuffer->GetReadOnlyDepthStencilView());

	// Prepare the blend, depth and sampler
	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthEnabled(), 0);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

	Quad* fsQuad = GetFullScreenQuad();
	
	// Render
	V_RETURN(fsQuad->Render(pd3dImmediateContext, _enableSun ? _sunEnabledPS : _sunDisabledPS));
	
	END_EVENT_D3D(L"");

	return S_OK;
}

HRESULT SkyPostProcess:: OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	// Load the shaders
	ID3DBlob* pBlob = NULL;

	D3D_SHADER_MACRO skyMacros[] = 
	{
		{ "SUN_ENABLED", "" },
		NULL,
	};

	skyMacros[0].Definition = "0";
	V_RETURN( CompileShaderFromFile( L"Sky.hlsl", "PS_Sky", "ps_4_0", skyMacros, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_sunDisabledPS));
	SAFE_RELEASE(pBlob);
	SET_DEBUG_NAME(_sunDisabledPS, "Sky post process (sun disabled) pixel shader");

	skyMacros[0].Definition = "1";
	V_RETURN( CompileShaderFromFile( L"Sky.hlsl", "PS_Sky", "ps_4_0", skyMacros, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_sunEnabledPS));
	SAFE_RELEASE(pBlob);
	SET_DEBUG_NAME(_sunEnabledPS, "Sky post process (sun enabled) pixel shader");

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
	SET_DEBUG_NAME(_skyProperties, "Sky post process properties buffer");

	return S_OK;
}

void SkyPostProcess::OnD3D11DestroyDevice()
{
	PostProcess::OnD3D11DestroyDevice();

	SAFE_RELEASE(_sunEnabledPS);
	SAFE_RELEASE(_sunDisabledPS);
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