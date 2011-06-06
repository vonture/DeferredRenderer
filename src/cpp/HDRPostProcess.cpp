#include "HDRPostProcess.h"
#include <cmath>

HDRPostProcess::HDRPostProcess()
	: _timeDelta(0.0f), _adaptationRate(4.0f), _keyValue(0.2f), _luminanceMapPS(NULL),
	  _toneMapPS(NULL)
{
	for (UINT i = 0; i < 2; i++)
	{
		_adaptLuminanceTextures[i] = NULL;
		_adaptLuminanceRTVs[i] = NULL;
		_adaptLuminanceSRVs[i] = NULL;
	}
}

HDRPostProcess::~HDRPostProcess()
{
}

HRESULT HDRPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
	ID3D11RenderTargetView* dstRTV, ID3D11DepthStencilView* dstDSV, GBuffer* gBuffer,
	LightBuffer* lightBuffer)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Save the old viewport
	D3D11_VIEWPORT vpOld[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
    UINT nViewPorts = 1;
    pd3dImmediateContext->RSGetViewports(&nViewPorts, vpOld);
		
	// Prepare all the settings and map them
	V_RETURN(pd3dImmediateContext->Map(_hdrPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_HDR_PROPERTIES* hdrProperties = (CB_HDR_PROPERTIES*)mappedResource.pData;

	hdrProperties->TimeDelta = _timeDelta;
	hdrProperties->KeyValue = _keyValue;
	hdrProperties->MipLevels = _mipLevels;
	hdrProperties->AdaptationRate = _adaptationRate;	

	pd3dImmediateContext->Unmap(_hdrPropertiesBuffer, 0);

	// Set all the device states
	ID3D11SamplerState* sampler = GetSamplerStates()->GetPoint();
	pd3dImmediateContext->PSSetSamplers(0, 1, &sampler);

	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);
		
	D3D11_VIEWPORT vp;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = _lumMapSize;
	vp.Height = _lumMapSize;
	vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;

	pd3dImmediateContext->RSSetViewports(1, &vp);

	// render the luminance
	pd3dImmediateContext->OMSetRenderTargets(1, &_adaptLuminanceRTVs[0], NULL);

	ID3D11ShaderResourceView* ppSRVLumMap[2] = { src, _adaptLuminanceSRVs[1] };
	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVLumMap);

	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_hdrPropertiesBuffer);

	V_RETURN(_fsQuad.Render(pd3dImmediateContext, _luminanceMapPS));

	// Re-apply the old viewport
	pd3dImmediateContext->RSSetViewports(nViewPorts, vpOld);

	// Tone map the final result
	pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, dstDSV);

	pd3dImmediateContext->GenerateMips(_adaptLuminanceSRVs[0]);

	ID3D11ShaderResourceView* ppSRVToneMap[2] = { src, _adaptLuminanceSRVs[0] };
	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVToneMap);
	
	V_RETURN(_fsQuad.Render(pd3dImmediateContext, _toneMapPS));

	// Unset the SRVs
	ID3D11ShaderResourceView* ppSRVNULL[2] = { NULL, NULL };
	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVNULL);
	
	// Swap the adapted luminance buffers from the previous render
	swapLuminanceBuffers();
	
	return S_OK;
}

void HDRPostProcess::swapLuminanceBuffers()
{
	ID3D11Texture2D* tmpTex = _adaptLuminanceTextures[0];
	ID3D11RenderTargetView* tmpRTV = _adaptLuminanceRTVs[0];
	ID3D11ShaderResourceView* tmpSRV = _adaptLuminanceSRVs[0];

	_adaptLuminanceTextures[0] = _adaptLuminanceTextures[1];
	_adaptLuminanceRTVs[0] = _adaptLuminanceRTVs[1];
	_adaptLuminanceSRVs[0] = _adaptLuminanceSRVs[1];

	_adaptLuminanceTextures[1] = tmpTex;
	_adaptLuminanceRTVs[1] = tmpRTV;
	_adaptLuminanceSRVs[1] = tmpSRV;
}

HRESULT HDRPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	PostProcess::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc);
	
	// Load the shaders
	ID3DBlob* pBlob = NULL;

	V_RETURN( CompileShaderFromFile( L"HDR.hlsl", "PS_LuminanceMap", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_luminanceMapPS));
	SAFE_RELEASE(pBlob);
	
	V_RETURN( CompileShaderFromFile( L"HDR.hlsl", "PS_ToneMap", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_toneMapPS));
	SAFE_RELEASE(pBlob);

	V_RETURN(_fsQuad.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	// Create the buffer
	D3D11_BUFFER_DESC bufferDesc =
	{
		sizeof(CB_HDR_PROPERTIES), //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_hdrPropertiesBuffer));

	return S_OK;
}

void HDRPostProcess::OnD3D11DestroyDevice()
{
	PostProcess::OnD3D11DestroyDevice();
			
	SAFE_RELEASE(_luminanceMapPS);
	SAFE_RELEASE(_toneMapPS);

	SAFE_RELEASE(_hdrPropertiesBuffer);

	_fsQuad.OnD3D11DestroyDevice();
}

HRESULT HDRPostProcess::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	
	V_RETURN(_fsQuad.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	// Calculate the luminance map size to be the largest power of two that is smaller than
	// the smallest back buffer dimension
	float minSize = (float)min(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
	_lumMapSize = pow(2, floor(log(minSize)/log(2.0f)));

	_mipLevels = (UINT)ceil(log((float)_lumMapSize) / log(2.0f)) + 1;

	// Create the textures
	D3D11_TEXTURE2D_DESC textureDesc = 
    {
        _lumMapSize,//pBackBufferSurfaceDesc->Width,//UINT Width;
        _lumMapSize,//pBackBufferSurfaceDesc->Height,//UINT Height;
        _mipLevels,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R32_FLOAT,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        D3D11_RESOURCE_MISC_GENERATE_MIPS//UINT MiscFlags;    
    };
	
	V_RETURN(pd3dDevice->CreateTexture2D(&textureDesc, NULL, &_adaptLuminanceTextures[0]));
	V_RETURN(pd3dDevice->CreateTexture2D(&textureDesc, NULL, &_adaptLuminanceTextures[1]));

	// Create the render target views
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = 
	{
        DXGI_FORMAT_R32_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_adaptLuminanceTextures[0], &rtvDesc, &_adaptLuminanceRTVs[0]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_adaptLuminanceTextures[1], &rtvDesc, &_adaptLuminanceRTVs[1]));

	// Create the shader resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = 
    {
        DXGI_FORMAT_R32_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	srvDesc.Texture2D.MipLevels = _mipLevels;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_adaptLuminanceTextures[0], &srvDesc, &_adaptLuminanceSRVs[0]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_adaptLuminanceTextures[1], &srvDesc, &_adaptLuminanceSRVs[1]));
	
	return S_OK;
}

void HDRPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();

	_fsQuad.OnD3D11ReleasingSwapChain();

	for (UINT i = 0; i < 2; i++)
	{
		SAFE_RELEASE(_adaptLuminanceTextures[i]);
		SAFE_RELEASE(_adaptLuminanceRTVs[i]);
		SAFE_RELEASE(_adaptLuminanceSRVs[i]);
	}
}