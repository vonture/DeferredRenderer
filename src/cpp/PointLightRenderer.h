#pragma once

#include "Defines.h"
#include "LightRenderer.h"
#include "ModelRenderer.h"
#include "ModelInstance.h"
#include "xnaCollision.h"

struct CB_POINTLIGHT_DEPTH_PROPERTIES
{
	XMMATRIX WorldView;
	float Direction;
	XMFLOAT2 CameraClips;
	float Padding;
};

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

struct CB_POINTLIGHT_SHADOW_PROPERTIES
{
	XMFLOAT2 CameraClips;
	XMFLOAT2 ShadowMapSize;
	float Bias;
	XMFLOAT3 Padding;
	XMMATRIX ShadowMatrix;
};

class PointLightRenderer : public LightRenderer<PointLight>
{
private:
	ID3D11VertexShader* _depthVS;
	ID3D11InputLayout* _depthInput;
	ID3D11Buffer* _depthPropertiesBuffer;

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _unshadowedPS;
	ID3D11PixelShader* _shadowedPS;	

	ID3D11Buffer* _modelPropertiesBuffer;
	ID3D11Buffer* _lightPropertiesBuffer;
	ID3D11Buffer* _cameraPropertiesBuffer;
	ID3D11Buffer* _shadowPropertiesBuffer;

	ModelRenderer _modelRenderer;
	ModelInstance _lightModel;
	ID3D11InputLayout* _lightInputLayout;

	static const UINT NUM_SHADOW_MAPS = 3;
	static const UINT SHADOW_MAP_SIZE = 4096;
	static const float BIAS;
	ID3D11Texture2D* _shadowMapTextures[NUM_SHADOW_MAPS];
	ID3D11DepthStencilView* _shadowMapDSVs[NUM_SHADOW_MAPS];
	ID3D11ShaderResourceView* _shadowMapSRVs[NUM_SHADOW_MAPS];
	XMMATRIX _shadowMatricies[NUM_SHADOW_MAPS];
	 
	HRESULT renderDepth(ID3D11DeviceContext* pd3dImmediateContext, PointLight* light,
		UINT shadowMapIdx, std::vector<ModelInstance*>* models, Camera* camera,
		BoundingBox* sceneBounds);

protected:
	UINT GetMaxShadowedLights() { return NUM_SHADOW_MAPS; }

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