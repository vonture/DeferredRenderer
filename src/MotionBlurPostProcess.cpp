#include "MotionBlurPostProcess.h"

MotionBlurPostProcess::MotionBlurPostProcess()
	: _propertiesBuffer(NULL)
{
}

MotionBlurPostProcess::~MotionBlurPostProcess()
{
}

HRESULT MotionBlurPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext,
	ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer,
	LightBuffer* lightBuffer)
{
	DXUT_BeginPerfEvent(D3DCOLOR_COLORVALUE(0.0f, 0.0f, 1.0f, 1.0f), L"Motion blur");

	DXUT_EndPerfEvent();
	return E_FAIL;
}

HRESULT MotionBlurPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, 
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	// Create the buffer
	D3D11_BUFFER_DESC bufferDesc =
	{
		sizeof(CB_MOTIONBLUR_PROPERTIES), //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_propertiesBuffer));

	return S_OK;
}

void MotionBlurPostProcess::OnD3D11DestroyDevice()
{
	PostProcess::OnD3D11DestroyDevice();

	SAFE_RELEASE(_propertiesBuffer);
}

HRESULT MotionBlurPostProcess::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice,
	IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void MotionBlurPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();
}