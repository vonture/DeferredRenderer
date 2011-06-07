#include "Renderer.h"

Renderer::Renderer() : _begun(false)
{
}

Renderer::~Renderer()
{
}

void Renderer::AddModel(ModelInstance* model)
{
	if (model && _begun)
	{
		_models.push_back(model);
	}
}

void Renderer::AddLight(DirectionalLight* light, bool shadowed)
{
	_directionalLightRenderer.Add(light, shadowed);
}

void Renderer::AddLight(PointLight* light, bool shadowed)
{
	_pointLightRenderer.Add(light, shadowed);
}

void Renderer::AddPostProcess(PostProcess* postProcess)
{
	_postProcesses.push_back(postProcess);
}

void Renderer::AddDebugText(WCHAR* text)
{
	if (text)
	{
		_debugText.push_back(text);
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
	_directionalLightRenderer.Clear();
	_pointLightRenderer.Clear();

	_postProcesses.clear();
	_postProcesses.push_back(&_combinePP);

	_boRenderer.Clear();

	return S_OK;
}

HRESULT Renderer::End(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, Camera* camera)
{
	if (!_begun)
	{
		return E_FAIL;
	}
	_begun = false;
	
	HRESULT hr;

	ID3D11RenderTargetView* pOrigRTV = NULL;
    ID3D11DepthStencilView* pOrigDSV = NULL;
    pd3dImmediateContext->OMGetRenderTargets( 1, &pOrigRTV, &pOrigDSV );

	// Calculate the scene bounds
	AxisAlignedBox sceneBounds;
	if (_models.size() > 0)
	{
		sceneBounds = _models[0]->GetAxisAlignedBox();
		for (UINT i = 0; i < _models.size(); i++)
		{
			AxisAlignedBox modelAABB = _models[i]->GetAxisAlignedBox();

			Collision::MergeAxisAlignedBoxes(&sceneBounds, &sceneBounds, &modelAABB);
		}
	}

	// render the shadow maps
	DXUT_BeginPerfEvent(D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f), L"Shadow Maps");
	V_RETURN(_directionalLightRenderer.RenderShadowMaps(pd3dImmediateContext, &_models, camera, &sceneBounds));
	V_RETURN(_pointLightRenderer.RenderShadowMaps(pd3dImmediateContext, &_models, camera, &sceneBounds));
	DXUT_EndPerfEvent();

	// Render the scene to the gbuffer
	DXUT_BeginPerfEvent(D3DCOLOR_COLORVALUE(0.0f, 0.0f, 1.0f, 1.0f), L"G-Buffer");
	V_RETURN(_gBuffer.SetRenderTargetsAndDepthStencil(pd3dImmediateContext));
	V_RETURN(_gBuffer.Clear(pd3dImmediateContext));

	V_RETURN(_modelRenderer.RenderModels(pd3dImmediateContext, &_models, camera));

	V_RETURN(_gBuffer.UnsetRenderTargetsAndDepthStencil(pd3dImmediateContext));
	DXUT_EndPerfEvent();

	// render the lights
	V_RETURN(_lightBuffer.SetRenderTargets(pd3dImmediateContext, _gBuffer.GetReadOnlyDepthStencilView()));
	V_RETURN(_lightBuffer.Clear(pd3dImmediateContext));

	DXUT_BeginPerfEvent(D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f), L"Lights");
	V_RETURN(_directionalLightRenderer.RenderLights(pd3dImmediateContext, camera, &_gBuffer));
	V_RETURN(_pointLightRenderer.RenderLights(pd3dImmediateContext, camera, &_gBuffer));
	DXUT_EndPerfEvent();

	V_RETURN(_lightBuffer.UnsetRenderTargetsAndDepthStencil(pd3dImmediateContext));

	DXUT_BeginPerfEvent(D3DCOLOR_COLORVALUE(0.0f, 0.0f, 1.0f, 1.0f), L"Post-Processes");
	// render the post processes	
	for (UINT i = 0; i < _postProcesses.size(); i++)
	{
		// calculate source resource view
		ID3D11ShaderResourceView* srcSRV = (i % 2 != 0) ? _ppShaderResourceViews[0] : _ppShaderResourceViews[1];

		// Calculate destination render target
		ID3D11RenderTargetView* dstRTV = (i % 2 == 0) ? _ppRenderTargetViews[0] : _ppRenderTargetViews[1];

		if (i == 0)
		{
			// First pass, no source texture, just the gbuffer and light buffer
			srcSRV = NULL;
		}
		if (i == _postProcesses.size() - 1)
		{
			// Last pass, render to the original rtv and dsv
			dstRTV = pOrigRTV;
		}

		// Render the post process
		V_RETURN(_postProcesses[i]->Render(pd3dImmediateContext, srcSRV, dstRTV, camera,
			&_gBuffer, &_lightBuffer));
	}
	DXUT_EndPerfEvent();

	// Render the bounding shapes
	if (_drawBoundingObjects)
	{
		DXUT_BeginPerfEvent(D3DCOLOR_COLORVALUE(0.0f, 1.0f, 1.0f, 1.0f), L"Bounding Objects");
		
		for (UINT i = 0; i < _models.size(); i++)
		{
			UINT meshCount = _models[i]->GetModelMeshCount();
			for (UINT j = 0; j < meshCount; j++)
			{
				OrientedBox obb = _models[i]->GetMeshOrientedBox(j);
				_boRenderer.Add(obb);
			}
		}
		for (UINT i = 0; i < _pointLightRenderer.GetCount(false); i++)
		{
			PointLight* light = _pointLightRenderer.GetLight(i, false);

			Sphere lightSphere;
			XMStoreFloat3(&lightSphere.Center, light->GetPosition());
			lightSphere.Radius = light->GetRadius();
		
			_boRenderer.Add(lightSphere);
		}
		for (UINT i = 0; i < _pointLightRenderer.GetCount(true); i++)
		{
			PointLight* light = _pointLightRenderer.GetLight(i, true);

			Sphere lightSphere;
			XMStoreFloat3(&lightSphere.Center, light->GetPosition());
			lightSphere.Radius = light->GetRadius();
		
			_boRenderer.Add(lightSphere);
		}
	
		XMMATRIX cameraProj = camera->GetProjection();

		Frustum cameraFrust;
		Collision::ComputeFrustumFromProjection(&cameraFrust, &cameraProj);
		XMStoreFloat3(&cameraFrust.Origin, camera->GetPosition());
		XMStoreFloat4(&cameraFrust.Orientation, camera->GetOrientation());

		_boRenderer.Add(cameraFrust);

		_boRenderer.Render(pd3dImmediateContext, camera);
		DXUT_EndPerfEvent();
	}

	SAFE_RELEASE(pOrigRTV);
    SAFE_RELEASE(pOrigDSV);
		
	return S_OK;
}

HRESULT Renderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
		
	// Call the sub content holders
	V_RETURN(_gBuffer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_lightBuffer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_modelRenderer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_combinePP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_directionalLightRenderer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_pointLightRenderer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_boRenderer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	return S_OK;
}

void Renderer::OnD3D11DestroyDevice()
{
	_gBuffer.OnD3D11DestroyDevice();
	_lightBuffer.OnD3D11DestroyDevice();
	_modelRenderer.OnD3D11DestroyDevice();
	_combinePP.OnD3D11DestroyDevice();
	_directionalLightRenderer.OnD3D11DestroyDevice();
	_pointLightRenderer.OnD3D11DestroyDevice();
	_boRenderer.OnD3D11DestroyDevice();
}

HRESULT Renderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
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
        DXGI_FORMAT_R32G32B32A32_FLOAT,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;    
    };

	V_RETURN(pd3dDevice->CreateTexture2D(&ppTextureDesc, NULL, &_ppTextures[0]));
	V_RETURN(pd3dDevice->CreateTexture2D(&ppTextureDesc, NULL, &_ppTextures[1]));

	// Create the shader resource views
	D3D11_SHADER_RESOURCE_VIEW_DESC ppSRVDesc = 
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	ppSRVDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_ppTextures[0], &ppSRVDesc, &_ppShaderResourceViews[0]));
	V_RETURN(pd3dDevice->CreateShaderResourceView(_ppTextures[1], &ppSRVDesc, &_ppShaderResourceViews[1]));

	// create the render target views
	D3D11_RENDER_TARGET_VIEW_DESC ppRTVDesc = 
	{
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        0,
        0
    };

	V_RETURN(pd3dDevice->CreateRenderTargetView(_ppTextures[0], &ppRTVDesc, &_ppRenderTargetViews[0]));
	V_RETURN(pd3dDevice->CreateRenderTargetView(_ppTextures[1], &ppRTVDesc, &_ppRenderTargetViews[1]));
	
	V_RETURN(_gBuffer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_lightBuffer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_modelRenderer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_combinePP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_directionalLightRenderer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_pointLightRenderer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_boRenderer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

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
	_modelRenderer.OnD3D11ReleasingSwapChain();
	_combinePP.OnD3D11ReleasingSwapChain();
	_directionalLightRenderer.OnD3D11ReleasingSwapChain();
	_pointLightRenderer.OnD3D11ReleasingSwapChain();
	_boRenderer.OnD3D11ReleasingSwapChain();
}