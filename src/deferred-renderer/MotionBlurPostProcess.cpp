#include "PCH.h"
#include "MotionBlurPostProcess.h"
#include "Logger.h"

MotionBlurPostProcess::MotionBlurPostProcess()
	: _propertiesBuffer(NULL)
{
	SetIsAdditive(false);
}

HRESULT MotionBlurPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext,
	ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer,
	ParticleBuffer* pBuffer, LightBuffer* lightBuffer)
{
	BEGIN_EVENT_D3D(L"Motion blur");

	END_EVENT_D3D(L"");
	return E_NOTIMPL;
}

HRESULT MotionBlurPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, 
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

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
	V_RETURN(SetDXDebugName(_propertiesBuffer, "Motion blur properties buffer"));

	return S_OK;
}

void MotionBlurPostProcess::OnD3D11DestroyDevice()
{
	PostProcess::OnD3D11DestroyDevice();

	SAFE_RELEASE(_propertiesBuffer);
}

HRESULT MotionBlurPostProcess::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
	IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void MotionBlurPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();
}