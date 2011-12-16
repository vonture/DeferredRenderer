#include "PCH.h"
#include "HDRPostProcess.h"
#include "DDSTextureLoader.h"
#include "Logger.h"
#include "PixelShaderLoader.h"
#include "TextureLoader.h"

HDRPostProcess::HDRPostProcess()
	: _timeDelta(0.0f), _lumMapSize(0), _mipLevels(0), _luminanceMapPS(NULL), _toneMapPS(NULL), 
	  _scalePS(NULL), _thresholdPS(NULL), _hBlurPS(NULL), _vBlurPS(NULL)
{
	SetIsAdditive(false);

	// Load some default values for the parameters
	SetTau(0.8f);
	SetWhiteLuminancePercentage(2.5f);
	SetBloomThreshold(0.75f);
	SetBloomMagnitude(0.6f);
	SetBloomBlurSigma(0.8f);
	SetExposureKey(0.6f);

	for (UINT i = 0; i < 2; i++)
	{
		_lumTextures[i] = NULL;
		_lumRTVs[i] = NULL;
		_lumSRVs[i] = NULL;
	}

	for (UINT i = 0; i < 3; i++)
	{
		_downScaleTextures[i] = NULL;
		_downScaleRTVs[i] = NULL;
		_downScaleSRVs[i] = NULL;
	}

	_blurTempTexture = NULL;
	_blurTempRTV = NULL;
	_blurTempSRV = NULL;

	_colorGradeSRV = NULL;
}

HRESULT HDRPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
	ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer)
{
	BEGIN_EVENT_D3D(L"HDR");
	
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
	hdrProperties->WhiteLuminancePercentage = _lumWhite;
	hdrProperties->ExposureKey = _exposureKey;
	hdrProperties->MipLevels = _mipLevels;
	hdrProperties->Tau = _tau;	
	hdrProperties->BloomThreshold = _bloomThreshold;
	hdrProperties->BloomMagnitude = _bloomMagnitude;
	hdrProperties->BloomBlurSigma = _bloomBlurSigma;
	hdrProperties->GaussianNumerator = 1.0f / sqrt(2.0f * Pi * _bloomBlurSigma * _bloomBlurSigma);
	hdrProperties->InverseSceneSize = _invSceneSize;

	pd3dImmediateContext->Unmap(_hdrPropertiesBuffer, 0);

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
		
	Quad* fsQuad = GetFullScreenQuad();

	D3D11_VIEWPORT vp;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = (float)_lumMapSize;
	vp.Height = (float)_lumMapSize;
	vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;

	pd3dImmediateContext->RSSetViewports(1, &vp);

	// render the luminance
	BEGIN_EVENT_D3D(L"Luminance map");
	pd3dImmediateContext->OMSetRenderTargets(1, &_lumRTVs[0], NULL);

	ID3D11ShaderResourceView* ppSRVLumMap[2] = { src, _lumSRVs[1] };
	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVLumMap);

	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_hdrPropertiesBuffer);

	V_RETURN(fsQuad->Render(pd3dImmediateContext, _luminanceMapPS));
	END_EVENT_D3D(L"");

	// Generate the mips for the luminance map
	BEGIN_EVENT_D3D(L"Generate MIPS");
	pd3dImmediateContext->GenerateMips(_lumSRVs[0]);
	END_EVENT_D3D(L"");

	// bloom threshold and downscale to 1/2
	BEGIN_EVENT_D3D(L"Bloom threshold");
	pd3dImmediateContext->OMSetRenderTargets(1, &_downScaleRTVs[0], NULL);

	ID3D11ShaderResourceView* ppSRVThresh[2] = { src, _lumSRVs[0] };
	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVThresh);

	vp.Width = vpOld[0].Width / 2.0f;
	vp.Height = vpOld[0].Height / 2.0f;
	pd3dImmediateContext->RSSetViewports(1, &vp);

	V_RETURN(fsQuad->Render(pd3dImmediateContext, _thresholdPS));
	END_EVENT_D3D(L"");

	// Downscale again to 1/4
	BEGIN_EVENT_D3D(L"Downscale to 1/4");
	pd3dImmediateContext->OMSetRenderTargets(1, &_downScaleRTVs[1], NULL);
	
	ID3D11ShaderResourceView* ppSRVDownScale1[2] = { _downScaleSRVs[0], NULL };
	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVDownScale1);

	vp.Width = vpOld[0].Width / 4.0f;
	vp.Height = vpOld[0].Height / 4.0f;
	pd3dImmediateContext->RSSetViewports(1, &vp);

	V_RETURN(fsQuad->Render(pd3dImmediateContext, _scalePS));
	END_EVENT_D3D(L"");

	// Downscale again to 1/8
	BEGIN_EVENT_D3D(L"Downscale to 1/8");
	pd3dImmediateContext->OMSetRenderTargets(1, &_downScaleRTVs[2], NULL);	
	pd3dImmediateContext->PSSetShaderResources(0, 1, &_downScaleSRVs[1]);

	vp.Width = vpOld[0].Width / 8.0f;
	vp.Height = vpOld[0].Height / 8.0f;
	pd3dImmediateContext->RSSetViewports(1, &vp);

	V_RETURN(fsQuad->Render(pd3dImmediateContext, _scalePS));
	END_EVENT_D3D(L"");

	// Blur the downscaled threshold image horizontally/vertically twice
	BEGIN_EVENT_D3D(L"Blur horizontal 1");
	pd3dImmediateContext->OMSetRenderTargets(1, &_blurTempRTV, NULL);
	pd3dImmediateContext->PSSetShaderResources(0, 1, &_downScaleSRVs[2]);
	V_RETURN(fsQuad->Render(pd3dImmediateContext, _hBlurPS));
	END_EVENT_D3D(L"");

	ID3D11ShaderResourceView* ppSRVNULL1[1] = { NULL };
	pd3dImmediateContext->PSSetShaderResources(0, 1, ppSRVNULL1);

	BEGIN_EVENT_D3D(L"Blur vertical 1");
	pd3dImmediateContext->OMSetRenderTargets(1, &_downScaleRTVs[2], NULL);
	pd3dImmediateContext->PSSetShaderResources(0, 1, &_blurTempSRV);
	V_RETURN(fsQuad->Render(pd3dImmediateContext, _vBlurPS));
	END_EVENT_D3D(L"");

	pd3dImmediateContext->PSSetShaderResources(0, 1, ppSRVNULL1);

	BEGIN_EVENT_D3D(L"Blur horizontal 2");
	pd3dImmediateContext->OMSetRenderTargets(1, &_blurTempRTV, NULL);
	pd3dImmediateContext->PSSetShaderResources(0, 1, &_downScaleSRVs[2]);
	V_RETURN(fsQuad->Render(pd3dImmediateContext, _hBlurPS));
	END_EVENT_D3D(L"");

	pd3dImmediateContext->PSSetShaderResources(0, 1, ppSRVNULL1);

	BEGIN_EVENT_D3D(L"Blur vertical 2");
	pd3dImmediateContext->OMSetRenderTargets(1, &_downScaleRTVs[2], NULL);
	pd3dImmediateContext->PSSetShaderResources(0, 1, &_blurTempSRV);
	V_RETURN(fsQuad->Render(pd3dImmediateContext, _vBlurPS));
	END_EVENT_D3D(L"");

	// Upscale to 1/4
	BEGIN_EVENT_D3D(L"Upscale to 1/4");
	pd3dImmediateContext->OMSetRenderTargets(1, &_downScaleRTVs[1], NULL);	
	pd3dImmediateContext->PSSetShaderResources(0, 1, &_downScaleSRVs[2]);

	vp.Width = vpOld[0].Width / 4.0f;
	vp.Height = vpOld[0].Height / 4.0f;	
	pd3dImmediateContext->RSSetViewports(1, &vp);

	V_RETURN(fsQuad->Render(pd3dImmediateContext, _scalePS));
	END_EVENT_D3D(L"");

	// Upscale to 1/2
	BEGIN_EVENT_D3D(L"Upscale to 1/2");
	pd3dImmediateContext->OMSetRenderTargets(1, &_downScaleRTVs[0], NULL);	
	pd3dImmediateContext->PSSetShaderResources(0, 1, &_downScaleSRVs[1]);

	vp.Width = vpOld[0].Width / 2.0f;
	vp.Height = vpOld[0].Height / 2.0f;	
	pd3dImmediateContext->RSSetViewports(1, &vp);

	V_RETURN(fsQuad->Render(pd3dImmediateContext, _scalePS));
	END_EVENT_D3D(L"");

	// Re-apply the old viewport
	pd3dImmediateContext->RSSetViewports(nViewPorts, vpOld);

	// Tone map the final result
	BEGIN_EVENT_D3D(L"Tone map and color grade");
	pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, NULL);

	ID3D11ShaderResourceView* ppSRVToneMap[4] = { src, _lumSRVs[0], _downScaleSRVs[0], _colorGradeSRV };
	pd3dImmediateContext->PSSetShaderResources(0, 4, ppSRVToneMap);
	
	V_RETURN(fsQuad->Render(pd3dImmediateContext, _toneMapPS));
	END_EVENT_D3D(L"");

	// Unset the SRVs
	ID3D11ShaderResourceView* ppSRVNULL4[4] = { NULL, NULL, NULL, NULL };
	pd3dImmediateContext->PSSetShaderResources(0, 4, ppSRVNULL4);
	
	// Swap the adapted luminance buffers from the previous render
	swapLuminanceBuffers();
	
	END_EVENT_D3D(L"");

	return S_OK;
}

void HDRPostProcess::swapLuminanceBuffers()
{
	ID3D11Texture2D* tmpTex = _lumTextures[0];
	ID3D11RenderTargetView* tmpRTV = _lumRTVs[0];
	ID3D11ShaderResourceView* tmpSRV = _lumSRVs[0];

	_lumTextures[0] = _lumTextures[1];
	_lumRTVs[0] = _lumRTVs[1];
	_lumSRVs[0] = _lumSRVs[1];

	_lumTextures[1] = tmpTex;
	_lumRTVs[1] = tmpRTV;
	_lumSRVs[1] = tmpSRV;
}

HRESULT HDRPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	
	// Load the shaders
	PixelShaderContent* psContent = NULL;
	char entryPoint[256];
	char debugName[512];	

	PixelShaderOptions psOpts = 
	{
		entryPoint, // const char* EntryPoint;
		NULL, // D3D_SHADER_MACRO* Defines;
		debugName // const char* DebugName;
	};

	// Lumiance map
	sprintf_s(entryPoint, "PS_LuminanceMap");
	sprintf_s(debugName, "HDR Luminance Map");
	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"HDR.hlsl", &psOpts, &psContent));
	
	_luminanceMapPS = psContent->PixelShader;
	_luminanceMapPS->AddRef();

	SAFE_RELEASE(psContent);


	// Tone map
	sprintf_s(entryPoint, "PS_ToneMap");
	sprintf_s(debugName, "HDR Tone Map");
	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"HDR.hlsl", &psOpts, &psContent));
	
	_toneMapPS = psContent->PixelShader;
	_toneMapPS->AddRef();

	SAFE_RELEASE(psContent);

	// Scale
	sprintf_s(entryPoint, "PS_Scale");
	sprintf_s(debugName, "HDR Scale");
	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"HDR.hlsl", &psOpts, &psContent));
	
	_scalePS = psContent->PixelShader;
	_scalePS->AddRef();

	SAFE_RELEASE(psContent);

	// Threshold
	sprintf_s(entryPoint, "PS_Threshold");
	sprintf_s(debugName, "HDR Threshold");
	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"HDR.hlsl", &psOpts, &psContent));
	
	_thresholdPS = psContent->PixelShader;
	_thresholdPS->AddRef();

	SAFE_RELEASE(psContent);

	// HBlur
	sprintf_s(entryPoint, "PS_BlurHorizontal");
	sprintf_s(debugName, "HDR Horizontal Blur");
	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"HDR.hlsl", &psOpts, &psContent));
	
	_hBlurPS = psContent->PixelShader;
	_hBlurPS->AddRef();

	SAFE_RELEASE(psContent);

	// VBlur
	sprintf_s(entryPoint, "PS_BlurVertical");
	sprintf_s(debugName, "HDR Vertical Blur");
	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"HDR.hlsl", &psOpts, &psContent));
	
	_vBlurPS = psContent->PixelShader;
	_vBlurPS->AddRef();

	SAFE_RELEASE(psContent);

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
	V_RETURN(SetDXDebugName(_hdrPropertiesBuffer, "HDR Properties Buffer"));

	// Load the color grading texture
	TextureContent* texContent = NULL;
	TextureOptions texOptions = 
	{
		true, // bool Generate3DFrom2D;
		debugName // const char* DebugName;
	};

	sprintf_s(debugName, "HDR Color Grade");
	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"ColorGrades\\color_grade_default.dds", &texOptions, &texContent));

	_colorGradeSRV = texContent->ShaderResourceView;
	_colorGradeSRV->AddRef();

	SAFE_RELEASE(texContent);

	_invSceneSize.x = 1.0f / pBackBufferSurfaceDesc->Width;
	_invSceneSize.y = 1.0f / pBackBufferSurfaceDesc->Height;

	return S_OK;
}

void HDRPostProcess::OnD3D11DestroyDevice()
{
	PostProcess::OnD3D11DestroyDevice();
			
	SAFE_RELEASE(_luminanceMapPS);
	SAFE_RELEASE(_toneMapPS);
	SAFE_RELEASE(_scalePS);
	SAFE_RELEASE(_thresholdPS);
	SAFE_RELEASE(_hBlurPS);
	SAFE_RELEASE(_vBlurPS);

	SAFE_RELEASE(_hdrPropertiesBuffer);

	SAFE_RELEASE(_colorGradeSRV);
}

HRESULT HDRPostProcess::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	
	// Create the luminance objects first

	// Calculate the luminance map size to be the largest power of two that is smaller than
	// the smallest back buffer dimension
	float minSize = (float)min(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
	_lumMapSize = (UINT)pow(2, floor(log(minSize)/log(2.0f)));

	_mipLevels = (UINT)ceil(log((float)_lumMapSize) / log(2.0f)) + 1;

	// Create the textures
	D3D11_TEXTURE2D_DESC lumTextureDesc = 
    {
        _lumMapSize,//pBackBufferSurfaceDesc->Width,//UINT Width;
        _lumMapSize,//pBackBufferSurfaceDesc->Height,//UINT Height;
        _mipLevels,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R16_FLOAT,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        D3D11_RESOURCE_MISC_GENERATE_MIPS//UINT MiscFlags;    
    };
	
	V_RETURN(pd3dDevice->CreateTexture2D(&lumTextureDesc, NULL, &_lumTextures[0]));
	V_RETURN(pd3dDevice->CreateTexture2D(&lumTextureDesc, NULL, &_lumTextures[1]));

	V_RETURN(SetDXDebugName(_lumTextures[0], "HDR Luminance Texture 0"));
	V_RETURN(SetDXDebugName(_lumTextures[1], "HDR Luminance Texture 1"));

	// Create the render target views
	D3D11_RENDER_TARGET_VIEW_DESC lumRTVDesc = 
	{
        DXGI_FORMAT_R16_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_lumTextures[0], &lumRTVDesc, &_lumRTVs[0]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_lumTextures[1], &lumRTVDesc, &_lumRTVs[1]));

	V_RETURN(SetDXDebugName(_lumRTVs[0], "HDR Luminance RTV 0"));
	V_RETURN(SetDXDebugName(_lumRTVs[1], "HDR Luminance RTV 1"));

	// Create the shader resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC lumSRVDesc = 
    {
        DXGI_FORMAT_R16_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	lumSRVDesc.Texture2D.MipLevels = _mipLevels;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_lumTextures[0], &lumSRVDesc, &_lumSRVs[0]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_lumTextures[1], &lumSRVDesc, &_lumSRVs[1]));

	V_RETURN(SetDXDebugName(_lumSRVs[0], "HDR Luminance SRV 0"));
	V_RETURN(SetDXDebugName(_lumSRVs[1], "HDR Luminance SRV 1"));
	
	// Create the downscale/blur temp objects
	// Create the textures
	D3D11_TEXTURE2D_DESC dsTextureDesc = 
    {
        1,//pBackBufferSurfaceDesc->Width,//UINT Width;
        1,//pBackBufferSurfaceDesc->Height,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R16G16B16A16_FLOAT,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0, //UINT MiscFlags;    
    };

	dsTextureDesc.Width = pBackBufferSurfaceDesc->Width / 2;
	dsTextureDesc.Height = pBackBufferSurfaceDesc->Height / 2;
	V_RETURN(pd3dDevice->CreateTexture2D(&dsTextureDesc, NULL, &_downScaleTextures[0]));

	V_RETURN(SetDXDebugName(_downScaleTextures[0], "HDR 1/2 Downsample Texture"));

	dsTextureDesc.Width = pBackBufferSurfaceDesc->Width / 4;
	dsTextureDesc.Height = pBackBufferSurfaceDesc->Height / 4;
	V_RETURN(pd3dDevice->CreateTexture2D(&dsTextureDesc, NULL, &_downScaleTextures[1]));

	V_RETURN(SetDXDebugName(_downScaleTextures[1], "HDR 1/4 Downsample Texture"));

	dsTextureDesc.Width = pBackBufferSurfaceDesc->Width / 8;
	dsTextureDesc.Height = pBackBufferSurfaceDesc->Height / 8;
	V_RETURN(pd3dDevice->CreateTexture2D(&dsTextureDesc, NULL, &_downScaleTextures[2]));
	V_RETURN(pd3dDevice->CreateTexture2D(&dsTextureDesc, NULL, &_blurTempTexture));

	V_RETURN(SetDXDebugName(_downScaleTextures[2], "HDR 1/8 Downsample Texture"));
	V_RETURN(SetDXDebugName(_blurTempTexture, "HDR Blur Temporary Texture"));

	// Create the render target views
	D3D11_RENDER_TARGET_VIEW_DESC dsRTVDesc = 
	{
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_downScaleTextures[0], &dsRTVDesc, &_downScaleRTVs[0]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_downScaleTextures[1], &dsRTVDesc, &_downScaleRTVs[1]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_downScaleTextures[2], &dsRTVDesc, &_downScaleRTVs[2]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_blurTempTexture, &dsRTVDesc, &_blurTempRTV));

	V_RETURN(SetDXDebugName(_downScaleRTVs[0], "HDR 1/2 Downsample RTV"));
	V_RETURN(SetDXDebugName(_downScaleRTVs[1], "HDR 1/4 Downsample RTV"));
	V_RETURN(SetDXDebugName(_downScaleRTVs[2], "HDR 1/8 Downsample RTV"));
	V_RETURN(SetDXDebugName(_blurTempRTV, "HDR Blur Temporary RTV"));

	// create the SRVs
	D3D11_SHADER_RESOURCE_VIEW_DESC dsSRVDesc = 
    {
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	dsSRVDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_downScaleTextures[0], &dsSRVDesc, &_downScaleSRVs[0]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_downScaleTextures[1], &dsSRVDesc, &_downScaleSRVs[1]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_downScaleTextures[2], &dsSRVDesc, &_downScaleSRVs[2]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_blurTempTexture, &dsSRVDesc, &_blurTempSRV));

	V_RETURN(SetDXDebugName(_downScaleSRVs[0], "HDR 1/2 Downsample SRV"));
	V_RETURN(SetDXDebugName(_downScaleSRVs[1], "HDR 1/4 Downsample SRV"));
	V_RETURN(SetDXDebugName(_downScaleSRVs[2], "HDR 1/8 Downsample SRV"));
	V_RETURN(SetDXDebugName(_blurTempSRV, "HDR Blur Temporary SRV"));

	_invSceneSize.x = 1.0f / pBackBufferSurfaceDesc->Width;
	_invSceneSize.y = 1.0f / pBackBufferSurfaceDesc->Height;

	return S_OK;
}

void HDRPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();

	for (UINT i = 0; i < 2; i++)
	{
		SAFE_RELEASE(_lumTextures[i]);
		SAFE_RELEASE(_lumRTVs[i]);
		SAFE_RELEASE(_lumSRVs[i]);
	}

	for (UINT i = 0; i < 3; i++)
	{
		SAFE_RELEASE(_downScaleTextures[i]);
		SAFE_RELEASE(_downScaleRTVs[i]);
		SAFE_RELEASE(_downScaleSRVs[i]);
	}

	SAFE_RELEASE(_blurTempTexture);
	SAFE_RELEASE(_blurTempRTV);
	SAFE_RELEASE(_blurTempSRV);
}