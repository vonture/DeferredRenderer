#include "PCH.h"
#include "SkyPostProcess.h"
#include "Logger.h"

const SkyPostProcess::SKY_TYPE SkyPostProcess::SKY_TYPES[SKY_TYPE_COUNT] = 
{
	{  4.0f, -0.70f,  0.0f, -1.0f, 0.00f, L"CIE Standard Overcast Sky, steep luminance gradation towards zenith, azimuthal uniformity" },
	{  4.0f, -0.70f,  2.0f, -1.5f, 0.15f, L"Overcast, with steep luminance gradation and slight brightening towards the sun" },
	{  1.1f, -0.80f,  0.0f, -1.0f, 0.00f, L"Overcast, moderately graded with azimuthal uniformity" },
	{  1.1f, -0.80f,  2.0f, -1.5f, 0.15f, L"Overcast, moderately graded and slight brightening towards the sun" },
	{  0.0f, -1.00f,  0.0f, -1.0f, 0.00f, L"Sky of uniform luminance" },
	{  0.0f, -1.00f,  2.0f, -1.5f, 0.15f, L"Partly cloudy sky, no gradation towards zenith, slight brightening towards the sun" },
	{  0.0f, -1.00f,  5.0f, -2.5f, 0.30f, L"Partly cloudy sky, no gradation towards zenith, brighter circumsolar region" },
	{  0.0f, -1.00f, 10.0f, -3.0f, 0.45f, L"Partly cloudy sky, no gradation towards zenith, distinct solar corona" },
	{ -1.0f, -0.55f,  2.0f, -1.5f, 0.15f, L"Partly cloudy, with the obscured sun" },
	{ -1.0f, -0.55f,  5.0f, -2.5f, 0.30f, L"Partly cloudy, with brighter circumsolar region" },
	{ -1.0f, -0.55f, 10.0f, -3.0f, 0.45f, L"White-blue sky with distinct solar corona" },
	{ -1.0f, -0.32f, 10.0f, -3.0f, 0.45f, L"CIE Standard Clear Sky, low illuminance turbidity" },
	{ -1.0f, -0.32f, 16.0f, -3.0f, 0.30f, L"CIE Standard Clear Sky, polluted atmosphere" },
	{ -1.0f, -0.15f, 16.0f, -3.0f, 0.30f, L"Cloudless turbid sky with broad solar corona" },
	{ -1.0f, -0.15f, 24.0f, -2.8f, 0.15f, L"White-blue turbid sky with broad solar corona" },
};

SkyPostProcess::SkyPostProcess()
	: _skyProperties(NULL)
{
	SetIsAdditive(true);

	for (UINT i = 0; i < SKY_TYPE_COUNT; i++)
	{
		_skyPSs[0][i] = NULL;
		_skyPSs[1][i] = NULL;
	}

	SetSunColor(XMFLOAT3(1.0f, 0.8f, 0.5f));
	SetSkyColor(XMFLOAT3(0.2f, 0.5f, 1.0f));
	SetSunDirection(XMFLOAT3(0.4f, 0.9f, 0.2f));
	SetSunWidth(0.05f);
	SetSunEnabled(true);
	SetSunIntensity(4.0f);
	SetSkyTypeIndex(11);
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
	V_RETURN(fsQuad->Render(pd3dImmediateContext, _skyPSs[_enableSun ? 1 : 0][_skyTypeIndex]));
	
	END_EVENT_D3D(L"");

	return S_OK;
}

HRESULT SkyPostProcess:: OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

	// Load the shaders
	ID3DBlob* pBlob = NULL;

	char a[16], b[16], c[16], d[16], e[16];
	D3D_SHADER_MACRO skyMacros[] = 
	{
		{ "SUN_ENABLED", "" },
		{ "A", a },
		{ "B", b },
		{ "C", c },
		{ "D", d },
		{ "E", e },
		NULL,
	};

	char debugName[256];
	for (UINT i = 0; i < SKY_TYPE_COUNT; i++)
	{
		sprintf_s(a, "%#ff", SKY_TYPES[i].A);
		sprintf_s(b, "%#ff", SKY_TYPES[i].B);
		sprintf_s(c, "%#ff", SKY_TYPES[i].C);
		sprintf_s(d, "%#ff", SKY_TYPES[i].D);
		sprintf_s(e, "%#ff", SKY_TYPES[i].E);

		skyMacros[0].Definition = "0";
		V_RETURN( CompileShaderFromFile( L"Sky.hlsl", "PS_Sky", "ps_4_0", skyMacros, &pBlob ) );   
		V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_skyPSs[0][i]));
		SAFE_RELEASE(pBlob);

		sprintf_s(debugName, "Sky post process (sun disabled, type = %u)", i);
		SET_DEBUG_NAME(_skyPSs[0][i], debugName);

		skyMacros[0].Definition = "1";
		V_RETURN( CompileShaderFromFile( L"Sky.hlsl", "PS_Sky", "ps_4_0", skyMacros, &pBlob ) );   
		V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_skyPSs[1][i]));
		SAFE_RELEASE(pBlob);
		
		sprintf_s(debugName, "Sky post process (sun enabled, type = %u)", i);
		SET_DEBUG_NAME(_skyPSs[1][i], debugName);
	}
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

	for (UINT i = 0; i < SKY_TYPE_COUNT; i++)
	{
		SAFE_RELEASE(_skyPSs[0][i]);
		SAFE_RELEASE(_skyPSs[1][i]);
	}
	SAFE_RELEASE(_skyProperties);
}

HRESULT SkyPostProcess::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void SkyPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();
}