#include "PCH.h"
#include "FilmGrainVignettePostProcess.h"

FilmGrainVignettePostProcess::FilmGrainVignettePostProcess()
	: _propertiesBuffer(NULL)
{
	for (UINT i = 0; i < 2; i++)
	{
		for (UINT j = 0; j < 2; j++)
		{
			_ps[i][j] = NULL;
		}
	}

	SetIsAdditive(false);

	SetFilmGrainEnabled(true);
	SetFilmGrainStrength(1.5f);
	SetTime(0.0);

	SetVignetteEnabled(true);
	SetVignetteStart(0.35f);
	SetVignetteStrength(0.5f);
}

HRESULT FilmGrainVignettePostProcess::Render(ID3D11DeviceContext* pd3dImmediateContext,
	ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer,
	ParticleBuffer* pBuffer ,LightBuffer* lightBuffer)
{
	BEGIN_EVENT_D3D(L"Film grain/vignette");

	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Map the properties and set them
	V_RETURN(pd3dImmediateContext->Map(_propertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_PROPERTIES* properties = (CB_PROPERTIES*)mappedResource.pData;

	properties->DeltaTime = (float)_totalTime;
	properties->FilmGrainStrength = _filmGrainStrength;
	properties->VignetteStart = _vignetteStart;
	properties->VignetteStrength = _vignetteStrength;

	pd3dImmediateContext->Unmap(_propertiesBuffer, 0);

	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_propertiesBuffer);

	// Set the sampler
	ID3D11SamplerState* samplers[1] = { GetSamplerStates()->GetPointWrap() };

	pd3dImmediateContext->PSSetSamplers(0, 1, samplers);

	// Set the other states
	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

	// Set the render target
	pd3dImmediateContext->OMSetRenderTargets(1, &dstRTV, NULL);

	// Set the resources
	ID3D11ShaderResourceView* srvs[1] = { src };
	pd3dImmediateContext->PSSetShaderResources(0, 1, srvs);	

	// Render the quad
	Quad* fsQuad = GetFullScreenQuad();

	ID3D11PixelShader* ps = _ps[_filmGrainEnabled ? 1 : 0][_vignetteEnabled ? 1 : 0]->PixelShader;
	V_RETURN(fsQuad->Render(pd3dImmediateContext, ps));

	// Unset the resources
	ID3D11ShaderResourceView* ppSRVNULL[2] = { NULL, NULL };
	pd3dImmediateContext->PSSetShaderResources(0, 2, ppSRVNULL);

	ID3D11Buffer* ppBuffNULL[1] = { NULL };
	pd3dImmediateContext->PSSetConstantBuffers(0, 1, ppBuffNULL);

	END_EVENT_D3D(L"");
	return S_OK;
}

HRESULT FilmGrainVignettePostProcess::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

	char psDebugName[256];
	D3D_SHADER_MACRO macros[] = 
	{
		{ "FILM_GRAIN_ENABLED", " " },
		{ "VIGNETTE_ENABLED", " " },
		NULL,
	};

	PixelShaderOptions psOpts =
	{
		"PS_FilmGrainVignette",	// const char* EntryPoint;
		macros,					// D3D_SHADER_MACRO* Defines;
		psDebugName,			// const char* DebugName;
	};

	for (UINT i = 0; i < 2; i++)
	{
		macros[0].Definition = i ? "1" : "0";
		for (UINT j = 0; j < 2; j++)
		{
			macros[1].Definition = j ? "1" : "0";

			sprintf_s(psDebugName, "Film grain/vignette (film grain = %u, vignette = %u)", i ,j);

			V_RETURN(pContentManager->LoadContent(pd3dDevice, L"FilmGrainVignette.hlsl", &psOpts, &_ps[i][j]));
		}
	}

	// Create the buffers
	D3D11_BUFFER_DESC bufferDesc =
	{
		sizeof(CB_PROPERTIES), //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_propertiesBuffer));
	V_RETURN(SetDXDebugName(_propertiesBuffer, "Film grain properties buffer"));
	
	return S_OK;
}

void FilmGrainVignettePostProcess::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
	PostProcess::OnD3D11DestroyDevice(pContentManager);

	for (UINT i = 0; i < 2; i++)
	{
		for (UINT j = 0; j < 2; j++)
		{
			SAFE_CM_RELEASE(pContentManager, _ps[i][j]);
		}
	}

	SAFE_RELEASE(_propertiesBuffer);
}

HRESULT FilmGrainVignettePostProcess::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(PostProcess::OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void FilmGrainVignettePostProcess::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{
	PostProcess::OnD3D11ReleasingSwapChain(pContentManager);
}