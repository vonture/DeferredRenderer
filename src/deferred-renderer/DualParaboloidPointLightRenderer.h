#pragma once

#include "PCH.h"
#include "Lights.h"
#include "LightRenderer.h"
#include "ModelInstance.h"

class DualParaboloidPointLightRenderer : public LightRenderer<PointLight>
{
private:
	ID3D11VertexShader* _depthVS[2]; // alphacutout disabled/enabled
	ID3D11InputLayout* _depthInput[2]; // alphacutout disabled/enabled
	ID3D11PixelShader* _depthPS; // alphacutout enabled
	ID3D11Buffer* _depthPropertiesBuffer;
	ID3D11Buffer* _alphaCutoutPropertiesBuffer;

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _unshadowedPS;
	ID3D11PixelShader* _shadowedPS;	
		
	ID3D11Buffer* _modelPropertiesBuffer;	
	ID3D11Buffer* _lightPropertiesBuffer;
	ID3D11Buffer* _cameraPropertiesBuffer;
	ID3D11Buffer* _shadowPropertiesBuffer;

	ModelInstance _lightModel;
	ID3D11InputLayout* _lightInputLayout;

	static const UINT NUM_SHADOW_MAPS = 3;
	static const UINT SHADOW_MAP_SIZE = 2048;
	static const float BIAS;
	ID3D11Texture2D* _shadowMapTextures[NUM_SHADOW_MAPS];
	ID3D11DepthStencilView* _shadowMapDSVs[NUM_SHADOW_MAPS];
	ID3D11ShaderResourceView* _shadowMapSRVs[NUM_SHADOW_MAPS];
	XMFLOAT4X4 _shadowMatricies[NUM_SHADOW_MAPS];
	 
	HRESULT renderDepth(ID3D11DeviceContext* pd3dImmediateContext, PointLight* light,
		UINT shadowMapIdx, std::vector<ModelInstance*>* models, Camera* camera,
		AxisAlignedBox* sceneBounds);

	struct CB_POINTLIGHT_ALPHACUTOUT_PROPERTIES
	{
		float AlphaThreshold;
		XMFLOAT3 Padding;
	};

	struct CB_POINTLIGHT_DEPTH_PROPERTIES
	{
		XMFLOAT4X4 WorldView;
		float Direction;
		XMFLOAT2 CameraClips;
		float Padding;
	};

	struct CB_POINTLIGHT_MODEL_PROPERTIES
	{
		XMFLOAT4X4 World;
		XMFLOAT4X4 WorldViewProjection;
	};

	struct CB_POINTLIGHT_LIGHT_PROPERTIES
	{
		XMFLOAT3 LightPosition;
		float LightRadius;
		XMFLOAT3 LightColor;
		float LightBrightness;
	};

	struct CB_POINTLIGHT_CAMERA_PROPERTIES
	{
		XMFLOAT4X4 InverseViewProjection;
		XMFLOAT4 CameraPosition;
	};

	struct CB_POINTLIGHT_SHADOW_PROPERTIES
	{
		XMFLOAT2 CameraClips;
		XMFLOAT2 ShadowMapSize;
		float Bias;
		XMFLOAT3 Padding;
		XMFLOAT4X4 ShadowMatrix;
	};

protected:
	UINT GetMaxShadowedLights() const { return NUM_SHADOW_MAPS; }

public:
	DualParaboloidPointLightRenderer();

	HRESULT RenderGeometryShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, std::vector<ModelInstance*>* models,
		Camera* camera, AxisAlignedBox* sceneBounds);
	HRESULT RenderGeometryLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera, GBuffer* gBuffer);

	HRESULT RenderParticleLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
		ParticleBuffer* gBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};