#pragma once

#include "PCH.h"
#include "IHasContent.h"
#include "ModelInstance.h"
#include "Camera.h"
#include "ShaderLoader.h"
#include "DeviceStates.h"
#include <vector>

using namespace std;

struct CB_MODEL_PROPERTIES
{
	XMFLOAT4X4 World;
	XMFLOAT4X4 WorldViewProjection;
};

class ModelRenderer : public IHasContent
{
private:
	ID3D11VertexShader* _meshVertexShader;
	ID3D11PixelShader* _meshPixelShader;
	ID3D11InputLayout* _meshInputLayout;
	
	ID3D11Buffer* _constantBuffer;

	DepthStencilStates _dsStates;
	SamplerStates _samplerStates;
	BlendStates _blendStates;
	RasterizerStates _rasterStates;

public:
	ModelRenderer();
	~ModelRenderer();

	HRESULT RenderModels(ID3D11DeviceContext* pd3dDeviceContext, vector<ModelInstance*>* instances, Camera* camera);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};