#pragma once

#include "Defines.h"
#include "LightRenderer.h"
#include "Light.h"
#include "FullscreenQuad.h"
#include "DeviceStates.h"
#include "ModelRenderer.h"

struct CB_DIRECTIONALLIGHT_LIGHT_PROPERTIES
{
	XMFLOAT3 LightDirection;
	float LightIntensity;
	XMFLOAT4 LightColor;
};

struct CB_DIRECTIONALLIGHT_CAMERA_PROPERTIES
{
	XMMATRIX InverseViewProjection;
	XMFLOAT4 CameraPosition;
};

struct CB_DIRECTIONALLIGHT_SHADOW_PROPERTIES
{
	XMFLOAT2 CameraClips;
	XMFLOAT2 ShadowMapSize;
	float CascadeSplits[4];
	XMMATRIX ShadowMatricies[4];
};

class DirectionalLightRenderer : public LightRenderer<DirectionalLight>
{
private:
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
	ID3D11RenderTargetView* _shadowMapRTVs[NUM_SHADOW_MAPS];
	ID3D11ShaderResourceView* _shadowMapSRVs[NUM_SHADOW_MAPS];
	ID3D11Texture2D* _shadowMapDSTexture;
	ID3D11DepthStencilView* _shadowMapDSView;
	XMMATRIX _shadowMatricies[NUM_SHADOW_MAPS][NUM_CASCADES];
	float _cascadeSplits[NUM_SHADOW_MAPS][NUM_CASCADES];

	HRESULT renderDepth(ID3D11DeviceContext* pd3dImmediateContext, DirectionalLight* dlight,
		UINT shadowMapIdx, std::vector<ModelInstance*>* models, Camera* camera,
		BoundingBox* sceneBounds);

protected:
	UINT GetMaxShadowedLights() { return NUM_SHADOW_MAPS; }

public:
	DirectionalLightRenderer();

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