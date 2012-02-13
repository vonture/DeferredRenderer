#pragma once

#include "PCH.h"
#include "LightRenderer.h"
#include "Lights.h"
#include "Camera.h"
#include "PostProcess.h"
#include "GeometryCombinePostProcess.h"
#include "ModelInstance.h"
#include "GBuffer.h"
#include "LightBuffer.h"
#include "ParticleBuffer.h"
#include "IHasContent.h"
#include "ModelRenderer.h"
#include "ParticleRenderer.h"
#include "xnaCollision.h"

class Renderer : public IHasContent
{
private:
	bool _begun;
	GBuffer _gBuffer;
	LightBuffer _lightBuffer;
	ParticleBuffer _particleBuffer;
	
	std::vector<ModelInstance*> _models;	
	std::vector<PostProcess*> _postProcesses;

	std::vector<ParticleSystemInstance*> _particleSystems;
		
	ModelRenderer _modelRenderer;
	ParticleRenderer _particleRenderer;
	GeometryCombinePostProcess _combinePP;

	ID3D11ShaderResourceView* _ppShaderResourceViews[2];
	ID3D11RenderTargetView* _ppRenderTargetViews[2];
	
	AmbientLight _ambientLight;

	typedef size_t LightTypeHash;
	std::map<LightTypeHash, LightRendererBase*> _lightRenderers;
	
	void swapPPBuffers();

public:
	Renderer();
	
	template <class lightType>
	void AddLightRenderer(LightRenderer<lightType>* renderer)
	{
		const type_info& info = typeid(lightType);
		_lightRenderers[info.hash_code()] = renderer;
	}

	template <class lightType>
	void AddLight(lightType* light, bool shadowed = true)
	{
		const type_info& info = typeid(lightType);
		LightTypeHash hash = info.hash_code();		
		
		if (_lightRenderers.find(hash) != _lightRenderers.end())
		{
			LightRenderer<lightType>* renderer = dynamic_cast<LightRenderer<lightType>*>(_lightRenderers[hash]);
			if (renderer)
			{
				renderer->Add(light, shadowed);
			}
		}
		else
		{
			LOG_ERROR(L"Renderer", L"Attempted to add a light type which does not have a renderer set.");
		}		
	}

	template<>
	void AddLight<AmbientLight>(AmbientLight* light, bool shadowed)
	{
		_ambientLight.MergeColor(light);
	}
	
	void AddModel(ModelInstance* model);
	void AddParticleSystem(ParticleSystemInstance* particleSystem);
	void AddPostProcess(PostProcess* postProcess);

	HRESULT Begin();
	HRESULT End(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera, Camera* clipCamera = NULL);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice(ContentManager* pContentManager);

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};