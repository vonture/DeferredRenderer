#pragma once

#include "Light.h"
#include "Camera.h"
#include "PostProcess.h"
#include "CombinePostProcess.h"
#include "ModelInstance.h"
#include "GBuffer.h"
#include "LightBuffer.h"
#include "IHasContent.h"
#include "ModelRenderer.h"
#include "DirectionalLightRenderer.h"
#include <vector>

using namespace std;

class Renderer : public IHasContent
{
private:
	bool _begun;
	GBuffer _gBuffer;
	LightBuffer _lightBuffer;
	vector<ModelInstance*> _models;
	vector<WCHAR*> _debugText;
	ModelRenderer _modelRenderer;
	CombinePostProcess _combinePP;
	DirectionalLightRenderer _directionalLightRenderer;

public:
	Renderer();
	~Renderer();

	void AddModel(ModelInstance* model);
	void AddLight(DirectionalLight* light, bool shadowed);
	void AddLight(PointLight* light, bool shadowed);
	void AddPostProcess(PostProcess* postProcess);
	void AddDebugText(WCHAR* text);

	HRESULT Begin();
	HRESULT End(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, Camera* camera);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};