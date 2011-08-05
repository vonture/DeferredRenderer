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

	size_t _ambientLightHash;
	std::map<size_t, LightRendererBase*> _lightRenderers;
	
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
		size_t hash = info.hash_code();		
		
		if (hash == _ambientLightHash)
		{
			// Special case for ambient lights
			AmbientLight* asAmbient = reinterpret_cast<AmbientLight*>(light);

			_ambientLight.Color.x += asAmbient->Color.x;
			_ambientLight.Color.y += asAmbient->Color.y;
			_ambientLight.Color.z += asAmbient->Color.z;			
		}
		else
		{
			// Regular light, look for its renderer
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
	}
	
	void AddModel(ModelInstance* model);
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