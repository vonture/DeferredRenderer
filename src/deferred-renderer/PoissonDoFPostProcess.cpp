#include "PCH.h"
#include "PoissonDoFPostProcess.h"
#include "Logger.h"

PoissonDoFPostProcess::PoissonDoFPostProcess()
	: _propertiesBuffer(NULL)
{
	SetIsAdditive(false);

	for (UINT i = 0; i < NUM_DOF_SAMPLE_COUNTS; i++)
	{
		_sampleBuffers[i] = NULL;
		_dofPSs[i] = NULL;
	}

	// default parameters
	SetFocalDistance(11.0f);
	SetFocalFalloffs(4.0f, 8.0f);
	SetCircleOfConfusionScale(3.0f);
	SetSampleCountIndex(1);
}

PoissonDoFPostProcess::~PoissonDoFPostProcess()
{
}

HRESULT PoissonDoFPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
	ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer)
{
	BEGIN_EVENT_D3D(L"DoF");

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
	properties->InvSceneResolution = XMFLOAT2(1.0f / _sceneWidth, 1.0f / _sceneHeight);

	pd3dImmediateContext->Unmap(_propertiesBuffer, 0);

	// Set the constant buffers
	ID3D11Buffer* cbs[2] = { _propertiesBuffer, _sampleBuffers[_sampleCountIndex] };
	pd3dImmediateContext->PSSetConstantBuffers(0, 2, cbs);

	// Set all the device states
	ID3D11SamplerState* samplers[1] = { GetSamplerStates()->GetPointClamp() };

	pd3dImmediateContext->PSSetSamplers(0, 1, samplers);

	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);
	
	// Set the render target
	pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, NULL);

	// Set the textures
	ID3D11ShaderResourceView* srvs[2] = { gBuffer->GetShaderResourceView(3), src };
	pd3dImmediateContext->PSSetShaderResources(0, 2, srvs);

	Quad* fsQuad = GetFullScreenQuad();

	// Render DOF
	V_RETURN(fsQuad->Render(pd3dImmediateContext, _dofPSs[_sampleCountIndex]));

	// Null the SRVs
	ID3D11ShaderResourceView* NULLSRV[2] = { NULL, NULL };
	pd3dImmediateContext->PSSetShaderResources(0, 2, NULLSRV);

	END_EVENT_D3D(L"");
	return S_OK;
}

HRESULT PoissonDoFPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	// Create the shaders
	ID3DBlob* pBlob = NULL;

	D3D_SHADER_MACRO sampleCountMacros[] = 
	{
		{ "DOF_SAMPLE_COUNT", "" },
		NULL,
	};

	char sampleCountString[6];
	char dofPSDebugName[256];
	for (UINT i = 0; i < NUM_DOF_SAMPLE_COUNTS; i++)
	{
		sprintf_s(sampleCountString, "%i", Poisson::GetDistributionSize(i));
		sampleCountMacros[0].Definition = sampleCountString;

		V_RETURN(CompileShaderFromFile( L"PoissonDoF.hlsl", "PS_DoF", "ps_4_0", sampleCountMacros, &pBlob));   
		V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_dofPSs[i]));
		SAFE_RELEASE(pBlob);

		sprintf_s(dofPSDebugName, "Poisson DoF (sample count = %s) PS", sampleCountString);
		SET_DEBUG_NAME(_dofPSs[i], dofPSDebugName);
	}

	// Create the buffers
	D3D11_BUFFER_DESC bufferDesc =
	{
		0, //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};
	
	D3D11_SUBRESOURCE_DATA sampleInitData;
	sampleInitData.pSysMem = NULL;
	sampleInitData.SysMemPitch = 0;
	sampleInitData.SysMemSlicePitch = 0;

	char bufferDebugName[256];
	for (UINT i = 0; i < NUM_DOF_SAMPLE_COUNTS; i++)
	{
		bufferDesc.ByteWidth = sizeof(XMFLOAT4) * Poisson::GetDistributionSize(i);
		sampleInitData.pSysMem = Poisson::GetDistribution(i);

		V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, &sampleInitData, &_sampleBuffers[i]));

		sprintf_s(bufferDebugName, "DoF sample buffer (samples = %u)", Poisson::GetDistributionSize(i));
		SET_DEBUG_NAME(_sampleBuffers[i], bufferDebugName);
	}

	bufferDesc.ByteWidth = sizeof(CB_DOF_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_propertiesBuffer));
	SET_DEBUG_NAME(_propertiesBuffer, "DoF Properties Buffer");

	// Store the resolution
	_sceneWidth = pBackBufferSurfaceDesc->Width;
	_sceneHeight = pBackBufferSurfaceDesc->Height;

	return S_OK;
}

void PoissonDoFPostProcess::OnD3D11DestroyDevice()
{
	PostProcess::OnD3D11DestroyDevice();
	
	SAFE_RELEASE(_propertiesBuffer);

	for (UINT i = 0; i < NUM_DOF_SAMPLE_COUNTS; i++)
	{
		SAFE_RELEASE(_sampleBuffers[i]);
		SAFE_RELEASE(_dofPSs[i]);
	}
}

HRESULT PoissonDoFPostProcess::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice,
	IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	
	// Store the resolution
	_sceneWidth = pBackBufferSurfaceDesc->Width;
	_sceneHeight = pBackBufferSurfaceDesc->Height;

	return S_OK;
}

void PoissonDoFPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();
}