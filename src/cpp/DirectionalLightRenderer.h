#pragma once

#include "Defines.h"
#include "LightRenderer.h"
#include "Light.h"
#include "FullscreenQuad.h"
#include "DeviceStates.h"
#include "ModelRenderer.h"

struct CB_DIRECTIONALLIGHT_DEPTH_PROPERTIES
{
	XMFLOAT4X4 WorldViewProjection;
};

struct CB_DIRECTIONALLIGHT_LIGHT_PROPERTIES
{
	_DECLSPEC_ALIGN_16_ XMFLOAT3 LightDirection;
	_DECLSPEC_ALIGN_16_ XMFLOAT3 LightColor;
};

struct CB_DIRECTIONALLIGHT_CAMERA_PROPERTIES
{
	XMFLOAT4X4 InverseViewProjection;
	XMFLOAT4 CameraPosition;
};

struct CB_DIRECTIONALLIGHT_SHADOW_PROPERTIES
{
	XMFLOAT2 CameraClips;
	XMFLOAT2 ShadowMapSize;
	float CascadeSplits[4];
	XMMATRIX ShadowMatricies[4];	
	XMMATRIX ShadowTexCoordTransforms[4];
};

class DirectionalLightRenderer : public LightRenderer<DirectionalLight>
{
private:
	ID3D11VertexShader* _depthVS;
	ID3D11InputLayout* _depthInput;
	ID3D11Buffer* _depthPropertiesBuffer;

	ID3D11PixelShader* _unshadowedPS;
	ID3D11PixelShader* _shadowedPS;
	ID3D11Buffer* _cameraPropertiesBuffer;
	ID3D11Buffer* _lightPropertiesBuffer;
	ID3D11Buffer* _shadowPropertiesBuffer;
	FullscreenQuad _fsQuad;

	ModelRenderer _modelRenderer;

	static const UINT NUM_SHADOW_MAPS = 3;
	static const UINT NUM_CASCADES = 4;
	static const UINT SHADOW_MAP_SIZE = 2048;
	static const float CASCADE_SPLITS[NUM_CASCADES];
	static const float BACKUP;
	static const float BIAS;
	ID3D11Texture2D* _shadowMapTextures[NUM_SHADOW_MAPS];
	ID3D11DepthStencilView* _shadowMapDSVs[NUM_SHADOW_MAPS];
	ID3D11ShaderResourceView* _shadowMapSRVs[NUM_SHADOW_MAPS];
	XMMATRIX _shadowMatricies[NUM_SHADOW_MAPS][NUM_CASCADES];
	XMMATRIX _shadowTexCoordTransforms[NUM_SHADOW_MAPS][NUM_CASCADES];
	float _cascadeSplits[NUM_SHADOW_MAPS][NUM_CASCADES];

	void ComputeNearAndFar(FLOAT& fNearPlane, FLOAT& fFarPlane, FXMVECTOR vLightCameraOrthographicMin, 
		FXMVECTOR vLightCameraOrthographicMax, XMVECTOR* pvPointsInCameraView);

	void CreateFrustumPointsFromCascadeInterval(float fCascadeIntervalBegin, FLOAT fCascadeIntervalEnd, 
		XMMATRIX &vProjection, XMVECTOR* pvCornerPointsWorld);

	HRESULT renderDepth(ID3D11DeviceContext* pd3dImmediateContext, DirectionalLight* dlight,
		UINT shadowMapIdx, std::vector<ModelInstance*>* models, Camera* camera,
		AxisAlignedBox* sceneBounds);

protected:
	UINT GetMaxShadowedLights() { return NUM_SHADOW_MAPS; }

public:
	DirectionalLightRenderer();

	HRESULT RenderShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, std::vector<ModelInstance*>* models,
		Camera* camera, AxisAlignedBox* sceneBounds);
	HRESULT RenderLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
		GBuffer* gBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};