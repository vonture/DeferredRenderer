#pragma once

#include "Defines.h"
#include "Lights.h"
#include "Camera.h"
#include "PostProcess.h"
#include "CombinePostProcess.h"
#include "ModelInstance.h"
#include "GBuffer.h"
#include "LightBuffer.h"
#include "IHasContent.h"
#include "ModelRenderer.h"
#include "SpotLightRenderer.h"
#include "BoundingObjectRenderer.h"
#include <vector>

namespace BoundingObjectDrawType
{
	enum
	{
		None			= 0,
		Lights			= (1 << 1),
		Models			= (1 << 2),
		ModelMeshes		= (1 << 3),
		CameraFrustums	= (1 << 4),
		All				= Lights | Models | ModelMeshes | CameraFrustums
	};
};

class Renderer : public IHasContent
{
private:
	bool _begun;
	GBuffer _gBuffer;
	LightBuffer _lightBuffer;
	std::vector<ModelInstance*> _models;
	std::vector<PostProcess*> _postProcesses;
	ModelRenderer _modelRenderer;
	CombinePostProcess _combinePP;

	UINT _boDrawTypes;
	BoundingObjectRenderer _boRenderer;
	
	ID3D11Texture2D* _ppTextures[2];
	ID3D11ShaderResourceView* _ppShaderResourceViews[2];
	ID3D11RenderTargetView* _ppRenderTargetViews[2];
	
	AmbientLight _ambientLight;
	LightRenderer<PointLight>* _pointLightRenderer;
	LightRenderer<DirectionalLight>* _directionalLightRenderer;
	LightRenderer<SpotLight>* _spotLightRenderer;

	void swapPPBuffers();

public:
	Renderer();
	~Renderer();

	void SetPointLightRenderer(LightRenderer<PointLight>* renderer);
	void SetDirectionalLightRenderer(LightRenderer<DirectionalLight>* renderer);
	void SetSpotLightRenderer(LightRenderer<SpotLight>* renderer);

	void AddModel(ModelInstance* model);
	void AddLight(AmbientLight* light);
	void AddLight(DirectionalLight* light, bool shadowed);
	void AddLight(PointLight* light, bool shadowed);
	void AddLight(SpotLight* light, bool shadowed);
	void AddPostProcess(PostProcess* postProcess);

	UINT GetBoundingObjectDrawTypes() const { return _boDrawTypes; }
	void SetBoundingObjectDrawTypes(UINT types) { _boDrawTypes = types; }

	HRESULT Begin();
	HRESULT End(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};