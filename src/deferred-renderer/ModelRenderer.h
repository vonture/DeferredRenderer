#pragma once

#include "PCH.h"
#include "IHasContent.h"
#include "ModelInstance.h"
#include "Camera.h"
#include "DeviceStates.h"
#include "PixelShaderLoader.h"
#include "VertexShaderLoader.h"

using namespace std;

struct CB_MODEL_PROPERTIES
{
	XMFLOAT4X4 ViewProjection;
	XMFLOAT4X4 PreviousViewProjection;
};

class ModelRenderer : public IHasContent
{
private:
	bool _alphaCutoutEnabled;
	float _alphaThreshold;

	VertexShaderContent* _meshVertexShader;

	// Pixel shaders for...
	// 1: DIFFUSE_MAPPED
	// 2: NORMAL_MAPPED
	// 3: SPECULAR_MAPPED
	// 4: ALPHA_CUTOUT_ENABLED
	PixelShaderContent* _meshPixelShader[2][2][2][2];
	
	ID3D11Buffer* _modelPropertiesBuffer;
	ID3D11Buffer* _alphaThresholdBuffer;

	static const UINT MAX_INSTANCES = 128;
	ID3D11Buffer* _instanceWorldVB;

	struct CB_MODEL_ALPHA_THRESHOLD
	{
		float AlphaThreshold;
		XMFLOAT3 Padding;
	};

	DepthStencilStates _dsStates;
	SamplerStates _samplerStates;
	BlendStates _blendStates;
	RasterizerStates _rasterStates;

public:
	ModelRenderer();

	bool GetAlphaCutoutEnabled() const { return _alphaCutoutEnabled; }
	void SetAlphaCutoutEnabled(bool enabled) { _alphaCutoutEnabled = enabled; }

	float GetAlphaThreshold() const { return _alphaThreshold; }
	void SetAlphaThreshold(float threshold) { _alphaThreshold = clamp(threshold, 0.0f, 1.0f); }

	HRESULT RenderModels(ID3D11DeviceContext* pd3dDeviceContext, vector<ModelInstance*>* instances,
		Camera* camera);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice(ContentManager* pContentManager);

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, 
		IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};