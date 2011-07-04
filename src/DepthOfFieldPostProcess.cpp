#include "DepthOfFieldPostProcess.h"

DepthOfFieldPostProcess::DepthOfFieldPostProcess()
	: _cocSizeTexture(NULL), _cocSizeRTV(NULL), _cocSizeSRV(NULL), _cocSizePS(NULL),
	  _dofBlurPS(NULL), _propertiesBuffer(NULL)
{
	// default parameters
	_focalDistance = 11.0f;
	_falloffStart = 4.0f;
	_falloffEnd = 8.0f;
	_cocScale = 0.05f;
}

DepthOfFieldPostProcess::~DepthOfFieldPostProcess()
{
}

HRESULT DepthOfFieldPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
	ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer)
{
	DXUT_BeginPerfEvent(D3DCOLOR_COLORVALUE(0.0f, 1.0f, 0.0f, 1.0f), L"DoF");

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
		GetSamplerStates()->GetPoint(),
		GetSamplerStates()->GetLinear(),
	};

	pd3dImmediateContext->PSSetSamplers(0, 2, samplers);

	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

	// render the CoC size
	DXUT_BeginPerfEvent(D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f), L"CoC size");
	pd3dImmediateContext->OMSetRenderTargets(1, &_cocSizeRTV, NULL);

	ID3D11ShaderResourceView* ppSRVCoCSize[1] = { gBuffer->GetShaderResourceView(3)};
	pd3dImmediateContext->PSSetShaderResources(0, 1, ppSRVCoCSize);

	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_propertiesBuffer);

	V_RETURN(_fsQuad.Render(pd3dImmediateContext, _cocSizePS));
	DXUT_EndPerfEvent();

	DXUT_EndPerfEvent();
	return S_OK;
}

HRESULT DepthOfFieldPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_fsQuad.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	// Load the shaders
	ID3DBlob* pBlob = NULL;

	V_RETURN( CompileShaderFromFile( L"DoF.hlsl", "PS_CoCSize", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_cocSizePS));
	SAFE_RELEASE(pBlob);
	SET_DEBUG_NAME(_cocSizePS, "CoC Size PS");

	V_RETURN( CompileShaderFromFile( L"DoF.hlsl", "PS_DoFBlur", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_dofBlurPS));
	SAFE_RELEASE(pBlob);
	SET_DEBUG_NAME(_dofBlurPS, "DoF Blur PS");

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
	
	_fsQuad.OnD3D11DestroyDevice();

	SAFE_RELEASE(_cocSizePS);
	SAFE_RELEASE(_dofBlurPS);
	SAFE_RELEASE(_propertiesBuffer);
}

HRESULT DepthOfFieldPostProcess::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice,
	IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));	
	V_RETURN(_fsQuad.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	// Create the textures
	D3D11_TEXTURE2D_DESC cocTextureDesc = 
    {
        pBackBufferSurfaceDesc->Width,//pBackBufferSurfaceDesc->Width,//UINT Width;
        pBackBufferSurfaceDesc->Height,//pBackBufferSurfaceDesc->Height,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R32_FLOAT,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        D3D11_RESOURCE_MISC_GENERATE_MIPS//UINT MiscFlags;    
    };

	V_RETURN(pd3dDevice->CreateTexture2D(&cocTextureDesc, NULL, &_cocSizeTexture));
	SET_DEBUG_NAME(_cocSizeTexture, "DoF CoC Size Texture");

	// Create the render target views
	D3D11_RENDER_TARGET_VIEW_DESC cocRTVDesc = 
	{
        DXGI_FORMAT_R32_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_cocSizeTexture, &cocRTVDesc, &_cocSizeRTV));
	SET_DEBUG_NAME(_cocSizeRTV, "DoF CoC Size RTV");

	D3D11_SHADER_RESOURCE_VIEW_DESC cocSRVDesc = 
    {
        DXGI_FORMAT_R32_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	cocSRVDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_cocSizeTexture, &cocSRVDesc, &_cocSizeSRV));
	SET_DEBUG_NAME(_cocSizeSRV, "DoF CoC Size SRV");

	return S_OK;
}

void DepthOfFieldPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();

	_fsQuad.OnD3D11ReleasingSwapChain();

	SAFE_RELEASE(_cocSizeTexture);
	SAFE_RELEASE(_cocSizeRTV);
	SAFE_RELEASE(_cocSizeSRV);
}