#pragma once

#include "PCH.h"
#include "LightRenderer.h"
#include "Lights.h"
#include "FullscreenQuad.h"
#include "DeviceStates.h"

class CascadedDirectionalLightRenderer : public LightRenderer<DirectionalLight>
{
private:
	// No Alpha cutout shaders
	ID3D11VertexShader* _depthVSNoAlpha;
	ID3D11InputLayout* _depthInputNoAlpha;

	// Alpha cutout shaders
	ID3D11VertexShader* _depthVSAlpha;
	ID3D11InputLayout* _depthInputAlpha;
	ID3D11PixelShader* _depthPSAlpha;
	ID3D11Buffer* _alphaCutoutProperties;

	ID3D11Buffer* _depthPropertiesBuffer;	

	ID3D11PixelShader* _unshadowedPS;
	ID3D11PixelShader* _shadowedPS;
	ID3D11Buffer* _cameraPropertiesBuffer;
	ID3D11Buffer* _lightPropertiesBuffer;
	ID3D11Buffer* _shadowPropertiesBuffer;
	FullscreenQuad _fsQuad;
	
	static const UINT NUM_SHADOW_MAPS = 3;
	static const UINT NUM_CASCADES = 4;
	static const UINT SHADOW_MAP_SIZE = 1024;
	static const float CASCADE_SPLITS[NUM_CASCADES];
	static const float BIAS;
	ID3D11Texture2D* _shadowMapTextures[NUM_SHADOW_MAPS];
	ID3D11DepthStencilView* _shadowMapDSVs[NUM_SHADOW_MAPS];
	ID3D11ShaderResourceView* _shadowMapSRVs[NUM_SHADOW_MAPS];
	XMFLOAT4X4 _shadowMatricies[NUM_SHADOW_MAPS][NUM_CASCADES];
	XMFLOAT4X4 _shadowTexCoordTransforms[NUM_SHADOW_MAPS][NUM_CASCADES];
	float _cascadeSplits[NUM_SHADOW_MAPS][NUM_CASCADES];

	void ComputeNearAndFar(FLOAT& fNearPlane, FLOAT& fFarPlane, FXMVECTOR vLightCameraOrthographicMin, 
		FXMVECTOR vLightCameraOrthographicMax, XMVECTOR* pvPointsInCameraView);

	void CreateAABBPoints(XMVECTOR* vAABBPoints, FXMVECTOR vCenter, FXMVECTOR vExtents);

	void CreateFrustumPointsFromCascadeInterval(float fCascadeIntervalBegin, FLOAT fCascadeIntervalEnd, 
		XMMATRIX &vProjection, XMVECTOR* pvCornerPointsWorld);

	HRESULT renderDepth(ID3D11DeviceContext* pd3dImmediateContext, DirectionalLight* dlight,
		UINT shadowMapIdx, std::vector<ModelInstance*>* models, Camera* camera,
		AxisAlignedBox* sceneBounds);
	
	struct CB_DIRECTIONALLIGHT_DEPTH_PROPERTIES
	{
		XMFLOAT4X4 WorldViewProjection;
	};

	struct CB_DIRECTIONALLIGHT_ALPHACUTOUT_PROPERTIES
	{
		float AlphaThreshold;
		XMFLOAT3 Padding;
	};

	struct CB_DIRECTIONALLIGHT_LIGHT_PROPERTIES
	{
		XMFLOAT3 LightDirection;
		float Padding;
		XMFLOAT3 LightColor;
		float LightBrightness;
	};

	struct CB_DIRECTIONALLIGHT_CAMERA_PROPERTIES
	{
		XMFLOAT4X4 InverseViewProjection;
		XMFLOAT3 CameraPosition;
		FLOAT Padding;
	};

	struct CB_DIRECTIONALLIGHT_SHADOW_PROPERTIES
	{
		XMFLOAT2 CameraClips;
		XMFLOAT2 ShadowMapSize;
		float CascadeSplits[4];
		XMFLOAT4X4 ShadowMatricies[4];	
		XMFLOAT4X4 ShadowTexCoordTransforms[4];
	};

protected:
	UINT GetMaxShadowedLights() const { return NUM_SHADOW_MAPS; }

public:
	CascadedDirectionalLightRenderer();

	HRESULT RenderShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, std::vector<ModelInstance*>* models,
		Camera* camera, AxisAlignedBox* sceneBounds);
	HRESULT RenderLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
		GBuffer* gBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};