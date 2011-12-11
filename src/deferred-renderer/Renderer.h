#pragma once

#include "PCH.h"
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
#include "xnaCollision.h"

namespace BoundingObjectDrawType
{
	enum
	{
		None			= 0,
		Lights			= (1 << 0),
		Models			= (1 << 1),
		ModelMeshes		= (1 << 2),
		CameraFrustums	= (1 << 3),
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
		
	ID3D11Texture2D* _ppTextures[2];
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
		_ambientLight.Color.x += light->Color.x;
		_ambientLight.Color.y += light->Color.y;
		_ambientLight.Color.z += light->Color.z;	
	}
	
	void AddModel(ModelInstance* model);
	void AddPostProcess(PostProcess* postProcess);

	HRESULT Begin();
	HRESULT End(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera, Camera* clipCamera = NULL);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};