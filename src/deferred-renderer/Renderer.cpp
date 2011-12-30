#include "PCH.h"
#include "Renderer.h"
#include "Logger.h"

Renderer::Renderer() 
	: _begun(false), _ambientLight(XMFLOAT3(0.0f, 0.0f, 0.0f), 1.0f)
{
	for (UINT i = 0; i < 2; i++)
	{
		_ppTextures[i] = NULL;
		_ppShaderResourceViews[i] = NULL;
		_ppRenderTargetViews[i] = NULL;
	}	
}

void Renderer::swapPPBuffers()
{
	ID3D11Texture2D* tmpTex = _ppTextures[0];
	ID3D11ShaderResourceView* tmpSRV = _ppShaderResourceViews[0];
	ID3D11RenderTargetView* tmpRTV = _ppRenderTargetViews[0];

	_ppTextures[0] = _ppTextures[1];
	_ppShaderResourceViews[0] = _ppShaderResourceViews[1];
	_ppRenderTargetViews[0] = _ppRenderTargetViews[1];

	_ppTextures[1] = tmpTex;
	_ppShaderResourceViews[1] = tmpSRV;
	_ppRenderTargetViews[1] = tmpRTV;
}

void Renderer::AddModel(ModelInstance* model)
{
	if (model && _begun)
	{
		_models.push_back(model);
	}
}

void Renderer::AddParticleSystem(ParticleSystemInstance* particleSystem)
{
	if (particleSystem && _begun)
	{
		_particleSystems.push_back(particleSystem);
	}
}


void Renderer::AddPostProcess(PostProcess* postProcess)
{
	if (postProcess && _begun)
	{
		_postProcesses.push_back(postProcess);
	}
}

HRESULT Renderer::Begin()
{
	if (_begun)
	{
		return E_FAIL;
	}
	_begun = true;

	_models.clear();	

	for (std::map<size_t, LightRendererBase*>::iterator it = _lightRenderers.begin(); it != _lightRenderers.end(); it++)
	{
		it->second->Clear();
	}

	_postProcesses.clear();
	_particleSystems.clear();
	_postProcesses.push_back(&_combinePP);

	_ambientLight.SetColor(XMFLOAT3(0.0f, 0.0f, 0.0f));
	_ambientLight.SetBrightness(0.0f);
	
	return S_OK;
}

HRESULT Renderer::End(ID3D11DeviceContext* pd3dImmediateContext, Camera* viewCamera, Camera* clipCamera)
{
	HRESULT hr;

	if (!_begun)
	{
		return E_FAIL;
	}
	_begun = false;

	if (!clipCamera)
	{
		clipCamera = viewCamera;
	}
	
	ID3D11RenderTargetView* pOrigRTV = NULL;
    ID3D11DepthStencilView* pOrigDSV = NULL;
    pd3dImmediateContext->OMGetRenderTargets( 1, &pOrigRTV, &pOrigDSV );

	AxisAlignedBox sceneBounds;
	BEGIN_EVENT(L"Calculate scene bounds");
	{
		if (_models.size() > 0)
		{
			sceneBounds = _models[0]->GetAxisAlignedBox();
			for (UINT i = 1; i < _models.size(); i++)
			{
				AxisAlignedBox modelAABB = _models[i]->GetAxisAlignedBox();

				Collision::MergeAxisAlignedBoxes(&sceneBounds, &sceneBounds, &modelAABB);
			}
		}
	}
	END_EVENT(L"");

	// render the shadow maps
	BEGIN_EVENT_D3D(L"Shadow Maps");
	{
		for (std::map<size_t, LightRendererBase*>::iterator it = _lightRenderers.begin(); it != _lightRenderers.end(); it++)
		{
			V_RETURN(it->second->RenderGeometryShadowMaps(pd3dImmediateContext, &_models, viewCamera, &sceneBounds));
		}
	}
	END_EVENT_D3D(L"");

	// Render the scene to the gbuffer
	BEGIN_EVENT_D3D(L"G-Buffer");
	{
		V_RETURN(_gBuffer.Clear(pd3dImmediateContext));

		ID3D11RenderTargetView* gBufferRTVs[3] = 
		{
			_gBuffer.GetDiffuseRTV(),
			_gBuffer.GetNormalRTV(),
			_gBuffer.GetVelocityRTV(),
		};	
		pd3dImmediateContext->OMSetRenderTargets(3, gBufferRTVs, _gBuffer.GetDepthDSV());

		V_RETURN(_modelRenderer.RenderModels(pd3dImmediateContext, &_models, viewCamera));
	}	
	END_EVENT_D3D(L"");

	// Render the particles
	BEGIN_EVENT_D3D(L"Particles");
	{
		V_RETURN(_particleBuffer.Clear(pd3dImmediateContext));

		ID3D11RenderTargetView* particleBufferRTVs[3] = 
		{
			_particleBuffer.GetDiffuseRTV(),
			_particleBuffer.GetNormalRTV(),
			NULL,
		};	
		pd3dImmediateContext->OMSetRenderTargets(3, particleBufferRTVs, _gBuffer.GetReadOnlyDepthDSV());

		V_RETURN(_particleRenderer.RenderParticles(pd3dImmediateContext, &_particleSystems, viewCamera, &_gBuffer));
	}
	END_EVENT_D3D(L"");

	V_RETURN(_lightBuffer.Clear(pd3dImmediateContext));
	_lightBuffer.SetAmbientColor(_ambientLight.GetColor());
	_lightBuffer.SetAmbientBrightness(_ambientLight.GetBrightness());

	// render the lights
	BEGIN_EVENT_D3D(L"Geometry Lights");
	{
		ID3D11RenderTargetView* lightBufferGetometryRTVs[2] = 
		{
			_lightBuffer.GetGeometryLightRTV(),
			NULL,
		};	
		pd3dImmediateContext->OMSetRenderTargets(2, lightBufferGetometryRTVs, _gBuffer.GetReadOnlyDepthDSV());
	
		for (std::map<size_t, LightRendererBase*>::iterator it = _lightRenderers.begin(); it != _lightRenderers.end(); it++)
		{
			V_RETURN(it->second->RenderGeometryLights(pd3dImmediateContext, viewCamera, &_gBuffer));
		}
	}
	END_EVENT_D3D(L"");
	
	BEGIN_EVENT_D3D(L"Particle Lights");
	{
		ID3D11RenderTargetView* lightBufferGetometryRTVs[1] = 
		{
			_lightBuffer.GetParticleLightRTV(),
		};	
		pd3dImmediateContext->OMSetRenderTargets(1, lightBufferGetometryRTVs, _gBuffer.GetReadOnlyDepthDSV());

		for (std::map<size_t, LightRendererBase*>::iterator it = _lightRenderers.begin(); it != _lightRenderers.end(); it++)
		{
			V_RETURN(it->second->RenderParticleLights(pd3dImmediateContext, viewCamera, &_particleBuffer));
		}

		ID3D11RenderTargetView* nullRTV[1] = { NULL };
		pd3dImmediateContext->OMSetRenderTargets(1, nullRTV, NULL);
	}
	END_EVENT_D3D(L"");
	
	BEGIN_EVENT_D3D(L"Post-Processes");
	{	
		// Find the final non-additive pp
		UINT lastNonAdditive = 0;
		for (UINT i = _postProcesses.size() - 1; i > 0; i--)
		{
			if (!_postProcesses[i]->GetIsAdditive())
			{
				lastNonAdditive = i;
				break;
			}
		}	
	
		// render the post processes	
		for (UINT i = 0; i < _postProcesses.size(); i++)
		{
			PostProcess* pp = _postProcesses[i];
			bool isAdditive = pp->GetIsAdditive();
		
			// calculate source resource view
			ID3D11ShaderResourceView* srcSRV = NULL;
			if (i > 0 && i <= lastNonAdditive)
			{
				srcSRV = _ppShaderResourceViews[0];
			}
		
			// Calculate destination render target
			ID3D11RenderTargetView* dstRTV = NULL;
			if (i >= lastNonAdditive)
			{
				dstRTV = pOrigRTV;
			}
			else if (isAdditive)
			{
				dstRTV = _ppRenderTargetViews[0];
			}
			else
			{
				dstRTV = _ppRenderTargetViews[1];
			}

			// Render the post process
			V_RETURN(pp->Render(pd3dImmediateContext, srcSRV, dstRTV, viewCamera, &_gBuffer, &_lightBuffer));

			if (!isAdditive)
			{
				swapPPBuffers();
			}
		}
	}
	END_EVENT_D3D(L"");

	SAFE_RELEASE(pOrigRTV);
    SAFE_RELEASE(pOrigDSV);
		
	return S_OK;
}

HRESULT Renderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
		
	// Call the sub content holders
	V_RETURN(_gBuffer.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	V_RETURN(_lightBuffer.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	V_RETURN(_particleBuffer.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	V_RETURN(_modelRenderer.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	V_RETURN(_particleRenderer.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	V_RETURN(_combinePP.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

	return S_OK;
}

void Renderer::OnD3D11DestroyDevice()
{
	_gBuffer.OnD3D11DestroyDevice();
	_lightBuffer.OnD3D11DestroyDevice();
	_particleBuffer.OnD3D11DestroyDevice();
	_modelRenderer.OnD3D11DestroyDevice();
	_particleRenderer.OnD3D11DestroyDevice();
	_combinePP.OnD3D11DestroyDevice();
}

HRESULT Renderer::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	// Create the pp textures
	D3D11_TEXTURE2D_DESC ppTextureDesc = 
    {
        pBackBufferSurfaceDesc->Width,//UINT Width;
        pBackBufferSurfaceDesc->Height,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R16G16B16A16_FLOAT,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;    
    };

	V_RETURN(pd3dDevice->CreateTexture2D(&ppTextureDesc, NULL, &_ppTextures[0]));
	V_RETURN(SetDXDebugName(_ppTextures[0], "Renderer post process texture 0"));

	V_RETURN(pd3dDevice->CreateTexture2D(&ppTextureDesc, NULL, &_ppTextures[1]));
	V_RETURN(SetDXDebugName(_ppTextures[1], "Renderer post process texture 1"));

	// Create the shader resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC ppSRVDesc = 
    {
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	ppSRVDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_ppTextures[0], &ppSRVDesc, &_ppShaderResourceViews[0]));
	V_RETURN(SetDXDebugName(_ppShaderResourceViews[0], "Renderer post process SRV 0"));

	V_RETURN(pd3dDevice->CreateShaderResourceView(_ppTextures[1], &ppSRVDesc, &_ppShaderResourceViews[1]));
	V_RETURN(SetDXDebugName(_ppShaderResourceViews[1], "Renderer post process SRV 1"));

	// create the render target views
	D3D11_RENDER_TARGET_VIEW_DESC ppRTVDesc = 
	{
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_ppTextures[0], &ppRTVDesc, &_ppRenderTargetViews[0]));
	V_RETURN(SetDXDebugName(_ppRenderTargetViews[0], "Renderer post process RTV 0"));

	V_RETURN(pd3dDevice->CreateRenderTargetView(_ppTextures[1], &ppRTVDesc, &_ppRenderTargetViews[1]));
	V_RETURN(SetDXDebugName(_ppRenderTargetViews[1], "Renderer post process RTV 1"));

	V_RETURN(_gBuffer.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_lightBuffer.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_particleBuffer.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_modelRenderer.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_particleRenderer.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_combinePP.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void Renderer::OnD3D11ReleasingSwapChain()
{	
	SAFE_RELEASE(_ppTextures[0]);
	SAFE_RELEASE(_ppTextures[1]);
	SAFE_RELEASE(_ppShaderResourceViews[0]);
	SAFE_RELEASE(_ppShaderResourceViews[1]);
	SAFE_RELEASE(_ppRenderTargetViews[0]);
	SAFE_RELEASE(_ppRenderTargetViews[1]);

	_gBuffer.OnD3D11ReleasingSwapChain();
	_lightBuffer.OnD3D11ReleasingSwapChain();
	_particleBuffer.OnD3D11ReleasingSwapChain();
	_modelRenderer.OnD3D11ReleasingSwapChain();
	_particleRenderer.OnD3D11ReleasingSwapChain();
	_combinePP.OnD3D11ReleasingSwapChain();
}