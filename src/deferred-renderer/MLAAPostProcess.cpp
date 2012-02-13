#include "PCH.h"
#include "MLAAPostProcess.h"
#include "Logger.h"

const UINT MLAAPostProcess::WEIGHT_TEXTURE_SIZES[NUM_WEIGHT_TEXTURES] = 
{ 
#ifdef ALL_PRESETS
	9, 17, 33, 65, 129
#else
	17, 33
#endif
};
const WCHAR* MLAAPostProcess::WEIGHT_TEXTURE_PATH = L"MLAA\\AreaMap";

MLAAPostProcess::MLAAPostProcess()
	: _edgeDetectRTV(NULL), _edgeDetectSRV(NULL), _blendWeightRTV(NULL), _blendWeightSRV(NULL),
	  _copyBackgroundPS(NULL), _neighborhoodBlendPS(NULL), _mlaaPropertiesBuffer(NULL),
	  _dsv(NULL), _textureWidth(1), _textureHeight(1)
{
	SetIsAdditive(false);

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
	SetDepthThreshold(0.25f);
	SetNormalThreshold(0.07f);
	SetLuminanceThreshold(0.008f);

	SetDepthDetectionEnabled(true);
	SetNormalDetectionEnabled(true);
	SetLuminanceDetectionEnabled(false);

	SetSearchSteps(16);
}

HRESULT MLAAPostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
	ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, ParticleBuffer* pBuffer,LightBuffer* lightBuffer)
{
	BEGIN_EVENT_D3D(L"MLAA");
	
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
		GetSamplerStates()->GetPointClamp(),
		GetSamplerStates()->GetLinearClamp(),
	};

	pd3dImmediateContext->PSSetSamplers(0, 2, samplers);
	
	float blendFactor[4] = {0, 0, 0, 0};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

	Quad* fsQuad = GetFullScreenQuad();

	// Render the edge detection
	BEGIN_EVENT_D3D(L"Edge Detection");

	pd3dImmediateContext->OMSetRenderTargets(1, &_edgeDetectRTV, _dsv);

	// Clear the RT since the edge detection makes use of discard
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pd3dImmediateContext->ClearRenderTargetView(_edgeDetectRTV, clearColor);
	pd3dImmediateContext->ClearDepthStencilView(_dsv, D3D11_CLEAR_STENCIL, 0.0f, 0x00);
	
	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetStencilReplace(), 0xFFFFFFFF);

	ID3D11ShaderResourceView* ppSRVEdgeDetect[3] = 
	{ 
		_depthDetect ? gBuffer->GetDepthSRV() : NULL,
		_normalDetect ?  gBuffer->GetNormalSRV() : NULL,
		_luminanceDetect ? gBuffer->GetDiffuseSRV() : NULL,
	};
	pd3dImmediateContext->PSSetShaderResources(0, 3, ppSRVEdgeDetect);

	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_mlaaPropertiesBuffer);

	ID3D11PixelShader* _curEdgePS = _edgeDetectPSs[_depthDetect ? 1 : 0][_normalDetect ? 1 : 0][_luminanceDetect ? 1 : 0]->PixelShader;
	V_RETURN(fsQuad->Render(pd3dImmediateContext, _curEdgePS));
	END_EVENT_D3D(L"");

	// Render blend weights
	BEGIN_EVENT_D3D(L"Calculate Blend Weights");

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
		_weightSRVs[weightTexIdx]->ShaderResourceView,
		NULL
	};
	pd3dImmediateContext->PSSetShaderResources(0, 3, ppSRVBlendWeight);

	V_RETURN(fsQuad->Render(pd3dImmediateContext, _blendWeightPSs[weightTexIdx]->PixelShader));

	END_EVENT_D3D(L"");

	// Copy src into dst	
	BEGIN_EVENT_D3D(L"Background Copy");

	pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, _dsv);

	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);
		
	ID3D11ShaderResourceView* ppSRVCopyAndNeighborhood[2] = 
	{
		src,
		_blendWeightSRV,
	};

	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVCopyAndNeighborhood);

	V_RETURN(fsQuad->Render(pd3dImmediateContext, _copyBackgroundPS->PixelShader));
	
	END_EVENT_D3D(L"");

	// Neighborhood blend	
	BEGIN_EVENT_D3D(L"Neighborhood Blend");
	
	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetStencilEqual(), 0xFFFFFFFF);

	V_RETURN(fsQuad->Render(pd3dImmediateContext, _neighborhoodBlendPS->PixelShader));

	END_EVENT_D3D(L"");
	
	// Clean up
	ID3D11ShaderResourceView* ppSRVNULL[2] = { NULL, NULL };
	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVNULL);

	END_EVENT_D3D(L"");
	return S_OK;
}

HRESULT MLAAPostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

	// Load the shaders	
	char debugName[512];
	D3D_SHADER_MACRO edgeDetectMacros[] = 
	{
		{ "EDGE_DETECT_DEPTH", "" },
		{ "EDGE_DETECT_NORMAL", "" },
		{ "EDGE_DETECT_COLOR", "" },
		NULL,
	};

	PixelShaderOptions edgeDetectPSOpts = 
	{
		"PS_EdgeDetect", // const char* EntryPoint;
		edgeDetectMacros, // D3D_SHADER_MACRO* Defines;
		debugName // const char* DebugName;
	};

	// Load the various edge detection pixel shaders
	for (UINT i = 0; i < 2; i++)
	{
		for (UINT j = 0; j < 2; j++)
		{
			for (UINT k = 0; k < 2; k++)
			{
				edgeDetectMacros[0].Definition = i ? "1" : "0";
				edgeDetectMacros[1].Definition = j ? "1" : "0";
				edgeDetectMacros[2].Definition = k ? "1" : "0";

				sprintf_s(debugName, "MLAA Edge Detect (depth = %u, normal = %u, luminance = %u)", i, j, k);

				V_RETURN(pContentManager->LoadContent(pd3dDevice, L"MLAA.hlsl", &edgeDetectPSOpts, &_edgeDetectPSs[i][j][k]));
			}
		}
	}

	char distanceStr[16];
	D3D_SHADER_MACRO blendWeightMacros[] = 
	{
		{ "MAX_DISTANCE", distanceStr },
		NULL,
	};

	PixelShaderOptions edgeWeightPSOpts = 
	{
		"PS_BlendWeight", // const char* EntryPoint;
		blendWeightMacros, // D3D_SHADER_MACRO* Defines;
		debugName // const char* DebugName;
	};

	TextureOptions texOptions = 
	{
		false, // bool Generate3DFrom2D;
		debugName // const char* DebugName;
	};
	
	WCHAR texturePath[MAX_PATH];
	for (UINT i = 0; i < NUM_WEIGHT_TEXTURES; i++)
	{
		// Edge weight shader
		sprintf_s(distanceStr, "%i", WEIGHT_TEXTURE_SIZES[i]);
		sprintf_s(debugName, "MLAA Blend Weight (MAX_DISTANCE = %i)", WEIGHT_TEXTURE_SIZES[i]);

		V_RETURN(pContentManager->LoadContent(pd3dDevice, L"MLAA.hlsl", &edgeWeightPSOpts, &_blendWeightPSs[i]));
		
		// Edge weight texture
		sprintf_s(debugName, "MLAA Blend Weight (Size = %i)", WEIGHT_TEXTURE_SIZES[i]);
		swprintf_s(texturePath, L"%s%i.dds", WEIGHT_TEXTURE_PATH, WEIGHT_TEXTURE_SIZES[i]);

		V_RETURN(pContentManager->LoadContent(pd3dDevice, texturePath, &texOptions, &_weightSRVs[i]));
	}

	PixelShaderOptions finalPassPSOpts = 
	{
		"", // const char* EntryPoint;
		NULL, // D3D_SHADER_MACRO* Defines;
		debugName // const char* DebugName;
	};

	// Copy shader
	finalPassPSOpts.EntryPoint = "PS_CopyBackground";
	sprintf_s(debugName, "MLAA Copy Background PS");
	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"MLAA.hlsl", &finalPassPSOpts, &_copyBackgroundPS));
	
	// Neighborhood blend
	finalPassPSOpts.EntryPoint = "PS_NeighborhoodBlend";
	sprintf_s(debugName, "MLAA Neighborhood Blend PS");
	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"MLAA.hlsl", &finalPassPSOpts, &_neighborhoodBlendPS));
	
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
	V_RETURN(SetDXDebugName(_mlaaPropertiesBuffer, "MLAA Properties Buffer"));

	return S_OK;
}

void MLAAPostProcess::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
	PostProcess::OnD3D11DestroyDevice(pContentManager);
	
	for (UINT i = 0; i < 2; i++)
	{
		for (UINT j = 0; j < 2; j++)
		{
			for (UINT k = 0; k < 2; k++)
			{
				SAFE_CM_RELEASE(pContentManager, _edgeDetectPSs[i][j][k]);
			}
		}
	}

	for (UINT i = 0; i < NUM_WEIGHT_TEXTURES; i++)
	{
		SAFE_CM_RELEASE(pContentManager, _weightSRVs[i]);
		SAFE_CM_RELEASE(pContentManager, _blendWeightPSs[i]);
	}
	
	SAFE_CM_RELEASE(pContentManager, _copyBackgroundPS);
	SAFE_CM_RELEASE(pContentManager, _neighborhoodBlendPS);

	SAFE_RELEASE(_mlaaPropertiesBuffer);
}

HRESULT MLAAPostProcess::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
	IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	
	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));	

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

	ID3D11Texture2D* dsTexture;
	V_RETURN(pd3dDevice->CreateTexture2D(&dsTextureDesc, NULL, &dsTexture));

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc =
	{
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		0,
	};
	dsvDesc.Texture2D.MipSlice = 0;

	V_RETURN(pd3dDevice->CreateDepthStencilView(dsTexture, &dsvDesc, &_dsv));
	V_RETURN(SetDXDebugName(_dsv, "MLAA Depth Stencil View"));

	SAFE_RELEASE(dsTexture);

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

	ID3D11Texture2D* edgeDetectTexture;
	V_RETURN(pd3dDevice->CreateTexture2D(&edgeDetectTextureDesc, NULL, &edgeDetectTexture));

	D3D11_SHADER_RESOURCE_VIEW_DESC edgeDetectSRVDesc = 
    {
        DXGI_FORMAT_R8G8_UNORM,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	edgeDetectSRVDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(edgeDetectTexture, &edgeDetectSRVDesc, &_edgeDetectSRV));
	V_RETURN(SetDXDebugName(_edgeDetectSRV, "MLAA Edge Detect SRV"));

	D3D11_RENDER_TARGET_VIEW_DESC edgeDetectRTVDesc = 
	{
        DXGI_FORMAT_R8G8_UNORM,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(edgeDetectTexture, &edgeDetectRTVDesc, &_edgeDetectRTV));
	V_RETURN(SetDXDebugName(_edgeDetectRTV, "MLAA Edge Detect RTV"));

	SAFE_RELEASE(edgeDetectTexture);

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

	ID3D11Texture2D* blendWeightTexture;
	V_RETURN(pd3dDevice->CreateTexture2D(&blendWeightTextureDesc, NULL, &blendWeightTexture));

	D3D11_SHADER_RESOURCE_VIEW_DESC blendWeightSRVDesc = 
    {
        DXGI_FORMAT_R8G8B8A8_UNORM,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	blendWeightSRVDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(blendWeightTexture, &blendWeightSRVDesc, &_blendWeightSRV));
	V_RETURN(SetDXDebugName(_blendWeightSRV, "MLAA Blend Weight SRV"));

	D3D11_RENDER_TARGET_VIEW_DESC blendWeightRTVDesc = 
	{
        DXGI_FORMAT_R8G8B8A8_UNORM,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(blendWeightTexture, &blendWeightRTVDesc, &_blendWeightRTV));
	V_RETURN(SetDXDebugName(_blendWeightRTV, "MLAA Blend Weight RTV"));

	SAFE_RELEASE(blendWeightTexture);

	return S_OK;
}

void MLAAPostProcess::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{
	PostProcess::OnD3D11ReleasingSwapChain(pContentManager);
		
	SAFE_RELEASE(_dsv);

	SAFE_RELEASE(_edgeDetectRTV);
	SAFE_RELEASE(_edgeDetectSRV);

	SAFE_RELEASE(_blendWeightRTV);
	SAFE_RELEASE(_blendWeightSRV);
}