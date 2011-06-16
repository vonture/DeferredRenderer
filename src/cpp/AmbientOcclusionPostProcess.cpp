#include "AmbientOcclusionPostProcess.h"

AmbientOcclusionPostProcess::AmbientOcclusionPostProcess()
	: _aoTexture(NULL), _aoRTV(NULL), _aoSRV(NULL), _blurTempTexture(NULL), _blurTempRTV(NULL), 
	  _blurTempSRV(NULL), _aoPS(NULL), _scalePS(NULL), _hBlurPS(NULL), _vBlurPS(NULL),
	  _aoPropertiesBuffer(NULL), _randomTexture(NULL), _randomSRV(NULL), _sampleDirectionsBuffer(NULL),
	  _compositePS(NULL)
{
	for (UINT i = 0; i < 2; i++)
	{
		_downScaleTextures[i] = NULL;
		_downScaleRTVs[i] = NULL;
		_downScaleSRVs[i] = NULL;
	}
	
	// Initialize some parameters to default values
	_sampleRadius = 0.3f;
	_blurSigma = 0.8f;
}

AmbientOcclusionPostProcess::~AmbientOcclusionPostProcess()
{
}

HRESULT AmbientOcclusionPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
	ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer)
{
	DXUT_BeginPerfEvent(D3DCOLOR_COLORVALUE(0.0f, 0.0f, 1.0f, 1.0f), L"SSAO");

	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Save the old viewport
	D3D11_VIEWPORT vpOld[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
    UINT nViewPorts = 1;
    pd3dImmediateContext->RSGetViewports(&nViewPorts, vpOld);
	
	// Prepare all the settings and map them
	V_RETURN(pd3dImmediateContext->Map(_aoPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_AO_PROPERTIES* aoProperties = (CB_AO_PROPERTIES*)mappedResource.pData;

	XMFLOAT4X4 fViewProj = camera->GetViewProjection();
	XMMATRIX viewProj = XMLoadFloat4x4(&fViewProj);

	XMVECTOR det;
	XMMATRIX invViewProj = XMMatrixInverse(&det, viewProj);
		
	XMStoreFloat4x4(&aoProperties->ViewProjection, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&aoProperties->InverseViewProjection, XMMatrixTranspose(invViewProj));
	aoProperties->SampleRadius = _sampleRadius;
	aoProperties->BlurSigma = _blurSigma;
	aoProperties->GaussianNumerator = 1.0f / sqrt(2.0f * Pi * _blurSigma * _blurSigma);
	aoProperties->CameraNearClip = camera->GetNearClip();
	aoProperties->CameraFarClip = camera->GetFarClip();

	pd3dImmediateContext->Unmap(_aoPropertiesBuffer, 0);

	// Set the constant buffers
	ID3D11Buffer* cbs[2] = 
	{
		_aoPropertiesBuffer,
		_sampleDirectionsBuffer
	};

	pd3dImmediateContext->PSSetConstantBuffers(0, 2, cbs);

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
		
	D3D11_VIEWPORT vp;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = vpOld[0].Width / 2.0f;
	vp.Height = vpOld[0].Height / 2.0f;
	vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;

	pd3dImmediateContext->RSSetViewports(1, &vp);

	// Render the SSAO
	pd3dImmediateContext->OMSetRenderTargets(1, &_aoRTV, NULL);
	//pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, NULL);

	ID3D11ShaderResourceView* ppSRVLumMap[3] =
	{ 
		gBuffer->GetShaderResourceView(1),
		gBuffer->GetShaderResourceView(3),
		_randomSRV
	};
	pd3dImmediateContext->PSSetShaderResources(0, 3, ppSRVLumMap);	

	V_RETURN(_fsQuad.Render(pd3dImmediateContext, _aoPS));
	
	// Down scale to 1/4
	pd3dImmediateContext->OMSetRenderTargets(1, &_downScaleRTVs[0], NULL);	
	pd3dImmediateContext->PSSetShaderResources(0, 1, &_aoSRV);

	vp.Width = vpOld[0].Width / 4.0f;
	vp.Height = vpOld[0].Height / 4.0f;
	pd3dImmediateContext->RSSetViewports(1, &vp);

	V_RETURN(_fsQuad.Render(pd3dImmediateContext, _scalePS));

	// Down scale to 1/8
	pd3dImmediateContext->OMSetRenderTargets(1, &_downScaleRTVs[1], NULL);	
	pd3dImmediateContext->PSSetShaderResources(0, 1, &_downScaleSRVs[0]);

	vp.Width = vpOld[0].Width / 8.0f;
	vp.Height = vpOld[0].Height / 8.0f;
	pd3dImmediateContext->RSSetViewports(1, &vp);

	V_RETURN(_fsQuad.Render(pd3dImmediateContext, _scalePS));

	// Blur
	pd3dImmediateContext->OMSetRenderTargets(1, &_blurTempRTV, NULL);
	pd3dImmediateContext->PSSetShaderResources(0, 1, &_downScaleSRVs[1]);
	V_RETURN(_fsQuad.Render(pd3dImmediateContext, _hBlurPS));

	ID3D11ShaderResourceView* ppSRVNULL1[1] = { NULL };
	pd3dImmediateContext->PSSetShaderResources(0, 1, ppSRVNULL1);

	pd3dImmediateContext->OMSetRenderTargets(1, &_downScaleRTVs[1], NULL);
	pd3dImmediateContext->PSSetShaderResources(0, 1, &_blurTempSRV);
	V_RETURN(_fsQuad.Render(pd3dImmediateContext, _vBlurPS));
	 
	// Upscale to 1/4
	pd3dImmediateContext->OMSetRenderTargets(1, &_downScaleRTVs[0], NULL);	
	pd3dImmediateContext->PSSetShaderResources(0, 1, &_downScaleSRVs[1]);

	vp.Width = vpOld[0].Width / 4.0f;
	vp.Height = vpOld[0].Height / 4.0f;	
	pd3dImmediateContext->RSSetViewports(1, &vp);

	V_RETURN(_fsQuad.Render(pd3dImmediateContext, _scalePS));

	// Upscale to 1/2
	pd3dImmediateContext->OMSetRenderTargets(1, &_aoRTV, NULL);	
	pd3dImmediateContext->PSSetShaderResources(0, 1, &_downScaleSRVs[0]);

	vp.Width = vpOld[0].Width / 2.0f;
	vp.Height = vpOld[0].Height / 2.0f;	
	pd3dImmediateContext->RSSetViewports(1, &vp);

	V_RETURN(_fsQuad.Render(pd3dImmediateContext, _scalePS));

	// Re-apply the old viewport
	pd3dImmediateContext->RSSetViewports(nViewPorts, vpOld);

	// Composite
	pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, NULL);

	ID3D11ShaderResourceView* ppSRVToneMap[2] = { src, _aoSRV };
	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVToneMap);

	V_RETURN(_fsQuad.Render(pd3dImmediateContext, _compositePS));

	// Unset the SRVs
	ID3D11ShaderResourceView* ppSRVNULL2[2] = { NULL, NULL };
	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVNULL2);

	DXUT_EndPerfEvent();

	return S_OK;
}

HRESULT AmbientOcclusionPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_fsQuad.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	// Load the shaders
	ID3DBlob* pBlob = NULL;
	
	V_RETURN( CompileShaderFromFile( L"SSAO.hlsl", "PS_SSAO", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_aoPS));
	SAFE_RELEASE(pBlob);
	
	V_RETURN( CompileShaderFromFile( L"SSAO.hlsl", "PS_Scale", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_scalePS));
	SAFE_RELEASE(pBlob);

	V_RETURN( CompileShaderFromFile( L"SSAO.hlsl", "PS_BlurHorizontal", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_hBlurPS));
	SAFE_RELEASE(pBlob);

	V_RETURN( CompileShaderFromFile( L"SSAO.hlsl", "PS_BlurVertical", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_vBlurPS));
	SAFE_RELEASE(pBlob);

	V_RETURN( CompileShaderFromFile( L"SSAO.hlsl", "PS_SSAO_Composite", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_compositePS));
	SAFE_RELEASE(pBlob);

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
	
	bufferDesc.ByteWidth = sizeof(CB_AO_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_aoPropertiesBuffer));
	
	// Create the random sample directions
	XMFLOAT4 sampleDirections[SSAO_SAMPLE_COUNT];	
	for (UINT i = 0; i < SSAO_SAMPLE_COUNT; i++)
	{
		float randPitch = (rand() / (float)RAND_MAX) * 2.0f * Pi;
		float randYaw = (rand() / (float)RAND_MAX) * 2.0f * Pi;
		float length = ((rand() / (float)RAND_MAX) * 0.9f) + 0.1f;

		XMVECTOR unitVec = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		XMMATRIX transform = XMMatrixMultiply(
			XMMatrixRotationRollPitchYaw(randPitch, randYaw, 0.0f),
			XMMatrixScaling(length, length, length));

		XMStoreFloat4(&sampleDirections[i], XMVector3TransformCoord(unitVec, transform));
	}

	D3D11_SUBRESOURCE_DATA directionInitData;
	directionInitData.pSysMem = &sampleDirections;
	directionInitData.SysMemPitch = 0;
	directionInitData.SysMemSlicePitch = 0;

	bufferDesc.ByteWidth = sizeof(CB_AO_SAMPLE_DIRECTIONS);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, &directionInitData, &_sampleDirectionsBuffer));	

	// The random texture and srv are not dependent on the back buffer, create them here
	D3D11_TEXTURE2D_DESC randomTextureDesc = 
    {
        RANDOM_TEXTURE_SIZE,//UINT Width;
        RANDOM_TEXTURE_SIZE,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R32G32B32_FLOAT,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;    
    };
	
	XMFLOAT3 randomData[RANDOM_TEXTURE_SIZE * RANDOM_TEXTURE_SIZE];
	for (UINT i = 0; i < RANDOM_TEXTURE_SIZE * RANDOM_TEXTURE_SIZE; i++)
	{
		XMVECTOR randomDir = XMVectorSet(
			((rand() / (float)RAND_MAX) * 2.0f) - 1.0f,
			((rand() / (float)RAND_MAX) * 2.0f) - 1.0f,
			((rand() / (float)RAND_MAX) * 2.0f) - 1.0f,
			0.0f);

		randomDir = XMVector3Normalize(randomDir);

		XMStoreFloat3(&randomData[i], randomDir);
	}

	D3D11_SUBRESOURCE_DATA randomInitData;
	randomInitData.pSysMem = &randomData;
	randomInitData.SysMemPitch = RANDOM_TEXTURE_SIZE * sizeof(XMFLOAT3);
	randomInitData.SysMemSlicePitch = 0;

	V_RETURN(pd3dDevice->CreateTexture2D(&randomTextureDesc, &randomInitData, &_randomTexture));

	// Create the shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC randomSRVDesc = 
    {
        DXGI_FORMAT_R32G32B32_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	randomSRVDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_randomTexture, &randomSRVDesc, &_randomSRV));

	return S_OK;
}

void AmbientOcclusionPostProcess::OnD3D11DestroyDevice()
{
	PostProcess::OnD3D11DestroyDevice();	
	
	SAFE_RELEASE(_randomTexture);
	SAFE_RELEASE(_randomSRV);

	SAFE_RELEASE(_aoPS);
	SAFE_RELEASE(_scalePS);
	SAFE_RELEASE(_hBlurPS);
	SAFE_RELEASE(_vBlurPS);
	SAFE_RELEASE(_compositePS);

	SAFE_RELEASE(_aoPropertiesBuffer);
	SAFE_RELEASE(_sampleDirectionsBuffer);

	_fsQuad.OnD3D11DestroyDevice();
}

HRESULT AmbientOcclusionPostProcess::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice,
	IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));	
	V_RETURN(_fsQuad.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	// Create the AO texture components
	D3D11_TEXTURE2D_DESC aoTextureDesc = 
    {
        pBackBufferSurfaceDesc->Width / 2,//UINT Width;
        pBackBufferSurfaceDesc->Height / 2,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R16_FLOAT,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;    
    };

	V_RETURN(pd3dDevice->CreateTexture2D(&aoTextureDesc, NULL, &_aoTexture));

	D3D11_RENDER_TARGET_VIEW_DESC aoRTVDesc = 
	{
        DXGI_FORMAT_R16_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_aoTexture, &aoRTVDesc, &_aoRTV));

	D3D11_SHADER_RESOURCE_VIEW_DESC aoSRVDesc = 
    {
        DXGI_FORMAT_R16_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	aoSRVDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_aoTexture, &aoSRVDesc, &_aoSRV));

	// Create the downscale and blur temp components
	D3D11_TEXTURE2D_DESC downScaleTextureDesc = 
    {
        1,//UINT Width;
        1,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R16_FLOAT,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;    
    };

	downScaleTextureDesc.Width = pBackBufferSurfaceDesc->Width / 4;
	downScaleTextureDesc.Height = pBackBufferSurfaceDesc->Height / 4;
	V_RETURN(pd3dDevice->CreateTexture2D(&downScaleTextureDesc, NULL, &_downScaleTextures[0]));
	
	downScaleTextureDesc.Width = pBackBufferSurfaceDesc->Width / 8;
	downScaleTextureDesc.Height = pBackBufferSurfaceDesc->Height / 8;
	V_RETURN(pd3dDevice->CreateTexture2D(&downScaleTextureDesc, NULL, &_downScaleTextures[1]));

	V_RETURN(pd3dDevice->CreateTexture2D(&downScaleTextureDesc, NULL, &_blurTempTexture));

	D3D11_RENDER_TARGET_VIEW_DESC downScaleRTVDesc = 
	{
        DXGI_FORMAT_R16_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_downScaleTextures[0], &downScaleRTVDesc, &_downScaleRTVs[0]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_downScaleTextures[1], &downScaleRTVDesc, &_downScaleRTVs[1]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_blurTempTexture, &downScaleRTVDesc, &_blurTempRTV));

	D3D11_SHADER_RESOURCE_VIEW_DESC downScaleSRVDesc = 
    {
        DXGI_FORMAT_R16_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	downScaleSRVDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_downScaleTextures[0], &downScaleSRVDesc, &_downScaleSRVs[0]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_downScaleTextures[1], &downScaleSRVDesc, &_downScaleSRVs[1]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_blurTempTexture, &downScaleSRVDesc, &_blurTempSRV));
	
	return S_OK;
}

void AmbientOcclusionPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();

	SAFE_RELEASE(_aoTexture);
	SAFE_RELEASE(_aoRTV);
	SAFE_RELEASE(_aoSRV);

	for (UINT i = 0; i < 2; i++)
	{
		SAFE_RELEASE(_downScaleTextures[i]);
		SAFE_RELEASE(_downScaleRTVs[i]);
		SAFE_RELEASE(_downScaleSRVs[i]);
	}

	SAFE_RELEASE(_blurTempTexture);
	SAFE_RELEASE(_blurTempRTV);
	SAFE_RELEASE(_blurTempSRV);

	_fsQuad.OnD3D11ReleasingSwapChain();
}