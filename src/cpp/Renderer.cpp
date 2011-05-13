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
}

void Renderer::AddPostProcess(PostProcess* postProcess)
{
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

	return S_OK;
}

HRESULT Renderer::End(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, Camera* camera)
{
	if (!_begun)
	{
		return E_FAIL;
	}
	_begun = false;

	ID3D11RenderTargetView* pOrigRTV = NULL;
    ID3D11DepthStencilView* pOrigDSV = NULL;
    pd3dImmediateContext->OMGetRenderTargets( 1, &pOrigRTV, &pOrigDSV );

	// Calculate the scene bounds
	bool first = true;
	BoundingBox sceneBounds;
	for (vector<ModelInstance*>::iterator i = _models.begin(); i != _models.end(); i++)
	{
		const BoundingBox* modelBB = (*i)->GetBounds();
		if (first)
		{
			sceneBounds = *modelBB;
			first = false;
		}
		else
		{
			BoundingBox::Combine(&sceneBounds, &sceneBounds, modelBB);
		}
	}

	// render the shadow maps
	_directionalLightRenderer.RenderShadowMaps(pd3dImmediateContext, &_models, camera, &sceneBounds);

	// Render the scene to the gbuffer
	_gBuffer.SetRenderTargets(pd3dImmediateContext);
	_gBuffer.Clear(pd3dImmediateContext);

	_modelRenderer.RenderModels(pd3dImmediateContext, &_models, camera);

	_gBuffer.UnsetRenderTargets(pd3dImmediateContext);

	// render the lights
	_lightBuffer.SetRenderTargets(pd3dImmediateContext);
	_lightBuffer.Clear(pd3dImmediateContext);

	_directionalLightRenderer.RenderLights(pd3dImmediateContext, camera, &_gBuffer);

	_lightBuffer.UnsetRenderTargets(pd3dImmediateContext);

	// render the post processes
	_combinePP.Render(pd3dImmediateContext, NULL, pOrigRTV, pOrigDSV, &_gBuffer, &_lightBuffer);
	
	SAFE_RELEASE( pOrigRTV );
    SAFE_RELEASE( pOrigDSV );
		
	return S_OK;
}

HRESULT Renderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(_gBuffer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_lightBuffer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_modelRenderer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_combinePP.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_directionalLightRenderer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	return S_OK;
}

void Renderer::OnD3D11DestroyDevice()
{
	_gBuffer.OnD3D11DestroyDevice();
	_lightBuffer.OnD3D11DestroyDevice();
	_modelRenderer.OnD3D11DestroyDevice();
	_combinePP.OnD3D11DestroyDevice();
	_directionalLightRenderer.OnD3D11DestroyDevice();
}

HRESULT Renderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(_gBuffer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_lightBuffer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_modelRenderer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_combinePP.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_directionalLightRenderer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void Renderer::OnD3D11ReleasingSwapChain()
{
	_gBuffer.OnD3D11ReleasingSwapChain();
	_lightBuffer.OnD3D11ReleasingSwapChain();
	_modelRenderer.OnD3D11ReleasingSwapChain();
	_combinePP.OnD3D11ReleasingSwapChain();
	_directionalLightRenderer.OnD3D11ReleasingSwapChain();
}