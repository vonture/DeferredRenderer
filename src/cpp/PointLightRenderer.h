#pragma once

#include "Defines.h"
#include "LightRenderer.h"
#include "ModelRenderer.h"
#include "ModelInstance.h"

struct CB_POINTLIGHT_MODEL_PROPERTIES
{
	XMMATRIX World;
	XMMATRIX WorldViewProjection;
};

struct CB_POINTLIGHT_LIGHT_PROPERTIES
{
	XMFLOAT3 LightPosition;
	float LightRadius;
	XMFLOAT3 LightColor;
	float LightIntensity;
};

struct CB_POINTLIGHT_CAMERA_PROPERTIES
{
	XMMATRIX InverseViewProjection;
	XMFLOAT4 CameraPosition;
};

class PointLightRenderer : public LightRenderer<PointLight>
{
private:
	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _unshadowedPS;
	ID3D11PixelShader* _shadowedPS;		

	ID3D11Buffer* _modelPropertiesBuffer;
	ID3D11Buffer* _lightPropertiesBuffer;
	ID3D11Buffer* _cameraPropertiesBuffer;
	
	ModelRenderer _modelRenderer;
	ModelInstance _lightModel;
	ID3D11InputLayout* _lightInputLayout;

protected:
	UINT GetMaxShadowedLights() { return 0; }

public:
	PointLightRenderer();

	HRESULT RenderShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, std::vector<ModelInstance*>* models,
		Camera* camera, BoundingBox* sceneBounds);
	HRESULT RenderLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
		GBuffer* gBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};