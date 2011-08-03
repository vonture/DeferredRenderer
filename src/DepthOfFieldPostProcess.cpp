#include "DepthOfFieldPostProcess.h"
#include "Logger.h"

DepthOfFieldPostProcess::DepthOfFieldPostProcess()
	: _propertiesBuffer(NULL)
{
	SetIsAdditive(false);

	// default parameters
	SetFocalDistance(11.0f);
	SetFocalFalloffs(4.0f, 8.0f);
	SetCircleOfConfusionScale(0.05f);
}

DepthOfFieldPostProcess::~DepthOfFieldPostProcess()
{
}

HRESULT DepthOfFieldPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
	ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer)
{
	BEGIN_EVENT(L"DoF");

	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Save the old viewport
	D3D11_VIEWPORT vpOld[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
    UINT nViewPorts = 1;
    pd3dImmediateContext->RSGetViewports(&nViewPorts, vpOld);

	// Prepare all the settings and map them
	V_RETURN(pd3dImmediateContext->Map(_propertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_DOF_PROPERTIES* properties = (CB_DOF_PROPERTIES*)mappedResource.pData;

	properties->CameraNearClip = camera->GetNearClip();
	properties->CameraFarClip = camera->GetFarClip();
	properties->FocalDistance = _focalDistance;
	properties->FocalFalloffNear = _falloffStart;
	properties->FocalFalloffFar = _falloffEnd;
	properties->CircleOfConfusionScale = _cocScale;

	pd3dImmediateContext->Unmap(_propertiesBuffer, 0);

	// Set all the device states
	ID3D11SamplerState* samplers[2] =
	{
		GetSamplerStates()->GetPointClamp(),
		GetSamplerStates()->GetLinearClamp(),
	};

	pd3dImmediateContext->PSSetSamplers(0, 2, samplers);

	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);
	
	//Quad* fsQuad = GetFullScreenQuad();

	// Render DOF (not implimented yet)

	END_EVENT();
	return E_NOTIMPL;
}

HRESULT DepthOfFieldPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	
	// Create the buffer
	D3D11_BUFFER_DESC bufferDesc =
	{
		sizeof(CB_DOF_PROPERTIES), //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_propertiesBuffer));
	SET_DEBUG_NAME(_propertiesBuffer, "DoF Properties Buffer");

	return S_OK;
}

void DepthOfFieldPostProcess::OnD3D11DestroyDevice()
{
	PostProcess::OnD3D11DestroyDevice();

	SAFE_RELEASE(_propertiesBuffer);
}

HRESULT DepthOfFieldPostProcess::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice,
	IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	
	return S_OK;
}

void DepthOfFieldPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();
}