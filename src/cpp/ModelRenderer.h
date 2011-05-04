#pragma once

#include "DXUT.h"
#include "IHasContent.h"
#include "ModelInstance.h"
#include "Camera.h"
#include "ShaderLoader.h"
#include <vector>

using namespace std;

struct CB_MODEL_PROPERTIES
{
	D3DXMATRIX World;
	D3DXMATRIX WorldViewProjection;
};

class ModelRenderer : public IHasContent
{
private:
	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;
	ID3D11InputLayout* _inputLayout;
	ID3D11Buffer* _constantBuffer;
	ID3D11SamplerState* _sampler;
public:
	ModelRenderer();
	~ModelRenderer();

	HRESULT RenderModel(ID3D11DeviceContext* pd3dDeviceContext,ModelInstance* instance, Camera* camera);
	HRESULT RenderModels(ID3D11DeviceContext* pd3dDeviceContext, vector<ModelInstance*> instances, Camera* camera);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};