#include "AntiAliasPostProcess.h"

const UINT AntiAliasPostProcess::WEIGHT_TEXTURE_SIZES[NUM_WEIGHT_TEXTURES] = { 9, 17, 33, 65, 129 };
const WCHAR* AntiAliasPostProcess::WEIGHT_TEXTURE_PATH = L"media\\MLAA\\AreaMap";

AntiAliasPostProcess::AntiAliasPostProcess()
	: _edgeDetectTexture(NULL), _edgeDetectRTV(NULL), _edgeDetectSRV(NULL),
	  _blendWeightTexture(NULL), _blendWeightRTV(NULL), _blendWeightSRV(NULL),
	  _copyBackgroundPS(NULL), _neighborhoodBlendPS(NULL),  _mlaaPropertiesBuffer(NULL),\
	  _dsTexture(NULL), _dsv(NULL), _textureWidth(1), _textureHeight(1)
{
	for (UINT i = 0; i < 2; i++)
	{
		for (UINT j = 0; j < 2; j++)
		{
			for (UINT k = 0; k < 2; k++)
			{
				_edgeDetectPSs[i][j][k] = NULL;
			}
		}
	}

	for (UINT i = 0; i < NUM_WEIGHT_TEXTURES; i++)
	{
		_weightSRVs[i] = NULL;
		_blendWeightPSs[i] = NULL;
	}

	// Define some default values for the properties
	_depthThreshold = 0.25f;
	_normalThreshold = 0.07f;
	_luminanceThreshold = 0.008f;

	_depthDetect = true;
	_normalDetect = true;
	_luminanceDetect = false;

	SetMaxSearchSteps(16);
}

AntiAliasPostProcess::~AntiAliasPostProcess()
{
}

HRESULT AntiAliasPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
	ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer)
{
	D3DPERF_BeginEvent(D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f), L"MLAA");
	
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Prepare all the settings and map them
	V_RETURN(pd3dImmediateContext->Map(_mlaaPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_MLAA_PROPERTIES* properties = (CB_MLAA_PROPERTIES*)mappedResource.pData;

	properties->InverseSceneSize = XMFLOAT2(1.0f / _textureWidth, 1.0f / _textureHeight);
	properties->DepthThreshold = _depthThreshold;
	properties->NormalThreshold = _normalThreshold;
	properties->LuminanceThreshold = _luminanceThreshold;
	properties->CameraNearClip = camera->GetNearClip();
	properties->CameraFarClip = camera->GetFarClip();
	properties->MaxSearchSteps = _maxSearchSteps;

	pd3dImmediateContext->Unmap(_mlaaPropertiesBuffer, 0);

	// Set all the device states
	ID3D11SamplerState* samplers[2] =
	{
		GetSamplerStates()->GetPoint(),
		GetSamplerStates()->GetLinear(),
	};

	pd3dImmediateContext->PSSetSamplers(0, 2, samplers);
	
	float blendFactor[4] = {0, 0, 0, 0};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

	Quad* fsQuad = GetFullScreenQuad();

	// Render the edge detection
	D3DPERF_BeginEvent(D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f), L"Edge Detection");

	pd3dImmediateContext->OMSetRenderTargets(1, &_edgeDetectRTV, _dsv);

	// Clear the RT since the edge detection makes use of discard
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pd3dImmediateContext->ClearRenderTargetView(_edgeDetectRTV, clearColor);
	pd3dImmediateContext->ClearDepthStencilView(_dsv, D3D11_CLEAR_STENCIL, 0.0f, 0x00);
	
	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetStencilReplace(), 0xFFFFFFFF);

	ID3D11ShaderResourceView* ppSRVEdgeDetect[3] = 
	{ 
		_depthDetect ? gBuffer->GetShaderResourceView(3) : NULL, // Depth
		_normalDetect ?  gBuffer->GetShaderResourceView(1) : NULL, // Normal
		_luminanceDetect ? gBuffer->GetShaderResourceView(0) : NULL, // Diffuse
	};
	pd3dImmediateContext->PSSetShaderResources(0, 3, ppSRVEdgeDetect);

	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_mlaaPropertiesBuffer);

	ID3D11PixelShader* _curEdgePS = _edgeDetectPSs[_depthDetect ? 1 : 0][_normalDetect ? 1 : 0][_luminanceDetect ? 1 : 0];
	V_RETURN(fsQuad->Render(pd3dImmediateContext, _curEdgePS));
	D3DPERF_EndEvent();

	// Render blend weights
	D3DPERF_BeginEvent(D3DCOLOR_COLORVALUE(0.0f, 1.0f, 0.0f, 1.0f), L"Calculate Blend Weights");

	// Determine which pixel shader and weight texture to use, max search steps must be <= than
	// (max_distance - 1) / 2
	UINT weightTexIdx = 0;
	for (UINT i = 0; i < NUM_WEIGHT_TEXTURES; i++)
	{
		if (_maxSearchSteps >= (WEIGHT_TEXTURE_SIZES[i] - 1) / 2)
		{
			weightTexIdx = i;
		}
	}

	pd3dImmediateContext->OMSetRenderTargets(1, &_blendWeightRTV, _dsv);
	pd3dImmediateContext->ClearRenderTargetView(_blendWeightRTV, clearColor);

	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetStencilEqual(), 0xFFFFFFFF);

	ID3D11ShaderResourceView* ppSRVBlendWeight[3] = 
	{
		_edgeDetectSRV,
		_weightSRVs[weightTexIdx],
		NULL
	};
	pd3dImmediateContext->PSSetShaderResources(0, 3, ppSRVBlendWeight);

	V_RETURN(fsQuad->Render(pd3dImmediateContext, _blendWeightPSs[weightTexIdx]));

	D3DPERF_EndEvent();

	// Copy src into dst	
	D3DPERF_BeginEvent(D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f), L"Background Copy");

	pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, _dsv);

	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);
		
	ID3D11ShaderResourceView* ppSRVCopyAndNeighborhood[2] = 
	{
		src,
		_blendWeightSRV,
	};

	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVCopyAndNeighborhood);

	V_RETURN(fsQuad->Render(pd3dImmediateContext, _copyBackgroundPS));
	
	D3DPERF_EndEvent();

	// Neighborhood blend	
	D3DPERF_BeginEvent(D3DCOLOR_COLORVALUE(0.0f, 1.0f, 0.0f, 1.0f), L"Neighborhood Blend");
	
	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetStencilEqual(), 0xFFFFFFFF);

	V_RETURN(fsQuad->Render(pd3dImmediateContext, _neighborhoodBlendPS));

	D3DPERF_EndEvent();
	
	// Clean up
	ID3D11ShaderResourceView* ppSRVNULL[2] = { NULL, NULL };
	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVNULL);

	D3DPERF_EndEvent();
	return S_OK;
}

HRESULT AntiAliasPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	// Load the shaders
	ID3DBlob* pBlob = NULL;
	
	D3D_SHADER_MACRO edgeDetectMacros[] = 
	{
		{ "EDGE_DETECT_DEPTH", "" },
		{ "EDGE_DETECT_NORMAL", "" },
		{ "EDGE_DETECT_COLOR", "" },
		NULL,
	};

	// Load the various edge detection pixel shaders
	char edgeDetectDebugName[MAX_PATH];
	for (UINT i = 0; i < 2; i++)
	{
		for (UINT j = 0; j < 2; j++)
		{
			for (UINT k = 0; k < 2; k++)
			{
				edgeDetectMacros[0].Definition = i ? "1" : "0";
				edgeDetectMacros[1].Definition = j ? "1" : "0";
				edgeDetectMacros[2].Definition = k ? "1" : "0";

				V_RETURN(CompileShaderFromFile(L"MLAA.hlsl", "PS_EdgeDetect", "ps_4_0", edgeDetectMacros, &pBlob));  
				V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_edgeDetectPSs[i][j][k]));
				SAFE_RELEASE(pBlob);
				
				sprintf_s(edgeDetectDebugName, "MLAA Edge Detect (%s%s%s) PS",
					i ? " Depth " : "", j ? " Normal " : "", k ? " Luminance " : "");
				SET_DEBUG_NAME(_edgeDetectPSs[i][j][k], edgeDetectDebugName);
			}
		}
	}

	D3D_SHADER_MACRO blendWeightMacros[] = 
	{
		{ "MAX_DISTANCE", "" },
		NULL,
	};

	char distanceStr[16];
	WCHAR texturePath[MAX_PATH];
	char blendWeightDebugName[MAX_PATH];
	for (UINT i = 0; i < NUM_WEIGHT_TEXTURES; i++)
	{
		sprintf_s(distanceStr, "%i", WEIGHT_TEXTURE_SIZES[i]);
		blendWeightMacros[0].Definition = distanceStr;

		V_RETURN(CompileShaderFromFile(L"MLAA.hlsl", "PS_BlendWeight", "ps_4_0", blendWeightMacros, &pBlob));   
		V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_blendWeightPSs[i]));
		SAFE_RELEASE(pBlob);
				
		sprintf_s(blendWeightDebugName, "MLAA Blend Weight (MAX_DISTANCE = %i) PS", WEIGHT_TEXTURE_SIZES[i]);
		SET_DEBUG_NAME(_blendWeightPSs[i], blendWeightDebugName);

		swprintf_s(texturePath, L"%s%i.dds", WEIGHT_TEXTURE_PATH, WEIGHT_TEXTURE_SIZES[i]);
		V_RETURN(DXUTFindDXSDKMediaFileCch(texturePath, MAX_PATH, texturePath));
		V_RETURN(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, texturePath, NULL, NULL, &_weightSRVs[i], NULL));

		sprintf_s(blendWeightDebugName, "MLAA Blend Weight (Size = %i) SRV", WEIGHT_TEXTURE_SIZES[i]);
		SET_DEBUG_NAME(_weightSRVs[i], blendWeightDebugName);
	}

	V_RETURN(CompileShaderFromFile(L"MLAA.hlsl", "PS_CopyBackground", "ps_4_0", NULL, &pBlob));   
    V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_copyBackgroundPS));
	SAFE_RELEASE(pBlob);
	SET_DEBUG_NAME(_copyBackgroundPS, "MLAA Copy Background PS");
	
	V_RETURN(CompileShaderFromFile(L"MLAA.hlsl", "PS_NeighborhoodBlend", "ps_4_0", NULL, &pBlob));   
    V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_neighborhoodBlendPS));
	SAFE_RELEASE(pBlob);
	SET_DEBUG_NAME(_neighborhoodBlendPS, "MLAA Neighborhood Blend PS");

	// create the buffer
	D3D11_BUFFER_DESC bufferDesc =
	{
		sizeof(CB_MLAA_PROPERTIES), //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};
	
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_mlaaPropertiesBuffer));
	SET_DEBUG_NAME(_mlaaPropertiesBuffer, "MLAA Properties Buffer");

	return S_OK;
}

void AntiAliasPostProcess::OnD3D11DestroyDevice()
{
	PostProcess::OnD3D11DestroyDevice();
	
	for (UINT i = 0; i < 2; i++)
	{
		for (UINT j = 0; j < 2; j++)
		{
			for (UINT k = 0; k < 2; k++)
			{
				SAFE_RELEASE(_edgeDetectPSs[i][j][k]);
			}
		}
	}

	for (UINT i = 0; i < NUM_WEIGHT_TEXTURES; i++)
	{
		SAFE_RELEASE(_weightSRVs[i]);
		SAFE_RELEASE(_blendWeightPSs[i]);
	}
	
	SAFE_RELEASE(_copyBackgroundPS);
	SAFE_RELEASE(_neighborhoodBlendPS);

	SAFE_RELEASE(_mlaaPropertiesBuffer);
}

HRESULT AntiAliasPostProcess::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice,
	IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	
	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));	

	_textureWidth = pBackBufferSurfaceDesc->Width;
	_textureHeight = pBackBufferSurfaceDesc->Height;

	// Create the depth stencil
	D3D11_TEXTURE2D_DESC dsTextureDesc = 
    {
        _textureWidth,//UINT Width;
        _textureHeight,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R24G8_TYPELESS,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_DEPTH_STENCIL,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;    
    };

	V_RETURN(pd3dDevice->CreateTexture2D(&dsTextureDesc, NULL, &_dsTexture));
	SET_DEBUG_NAME(_dsTexture, "MLAA Depth Stencil Texture");

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc =
	{
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		0,
	};
	dsvDesc.Texture2D.MipSlice = 0;

	V_RETURN(pd3dDevice->CreateDepthStencilView(_dsTexture, &dsvDesc, &_dsv));
	SET_DEBUG_NAME(_dsv, "MLAA Depth Stencil View");

	// Create the edge detection objects
	D3D11_TEXTURE2D_DESC edgeDetectTextureDesc = 
    {
        _textureWidth,//UINT Width;
        _textureHeight,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R8G8_UNORM,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;    
    };

	V_RETURN(pd3dDevice->CreateTexture2D(&edgeDetectTextureDesc, NULL, &_edgeDetectTexture));
	SET_DEBUG_NAME(_edgeDetectTexture, "MLAA Edge Detect Texture");

	D3D11_SHADER_RESOURCE_VIEW_DESC edgeDetectSRVDesc = 
    {
        DXGI_FORMAT_R8G8_UNORM,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	edgeDetectSRVDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_edgeDetectTexture, &edgeDetectSRVDesc, &_edgeDetectSRV));
	SET_DEBUG_NAME(_edgeDetectSRV, "MLAA Edge Detect SRV");

	D3D11_RENDER_TARGET_VIEW_DESC edgeDetectRTVDesc = 
	{
        DXGI_FORMAT_R8G8_UNORM,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_edgeDetectTexture, &edgeDetectRTVDesc, &_edgeDetectRTV));
	SET_DEBUG_NAME(_edgeDetectRTV, "MLAA Edge Detect RTV");

	// Create the blend weight objects
	D3D11_TEXTURE2D_DESC blendWeightTextureDesc = 
    {
        _textureWidth,//UINT Width;
        _textureHeight,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R8G8B8A8_UNORM,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;    
    };

	V_RETURN(pd3dDevice->CreateTexture2D(&blendWeightTextureDesc, NULL, &_blendWeightTexture));
	SET_DEBUG_NAME(_blendWeightTexture, "MLAA Blend Weight Texture");

	D3D11_SHADER_RESOURCE_VIEW_DESC blendWeightSRVDesc = 
    {
        DXGI_FORMAT_R8G8B8A8_UNORM,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	blendWeightSRVDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_blendWeightTexture, &blendWeightSRVDesc, &_blendWeightSRV));
	SET_DEBUG_NAME(_blendWeightSRV, "MLAA Blend Weight SRV");

	D3D11_RENDER_TARGET_VIEW_DESC blendWeightRTVDesc = 
	{
        DXGI_FORMAT_R8G8B8A8_UNORM,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_blendWeightTexture, &blendWeightRTVDesc, &_blendWeightRTV));
	SET_DEBUG_NAME(_blendWeightRTV, "MLAA Blend Weight RTV");

	return S_OK;
}

void AntiAliasPostProcess::OnD3D11ReleasingSwapChain()
{
	PostProcess::OnD3D11ReleasingSwapChain();
		
	SAFE_RELEASE(_dsTexture);
	SAFE_RELEASE(_dsv);

	SAFE_RELEASE(_edgeDetectTexture);
	SAFE_RELEASE(_edgeDetectRTV);
	SAFE_RELEASE(_edgeDetectSRV);

	SAFE_RELEASE(_blendWeightTexture);
	SAFE_RELEASE(_blendWeightRTV);
	SAFE_RELEASE(_blendWeightSRV);
}