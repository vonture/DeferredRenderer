#pragma once

#include "LightRenderer.h"
#include "Light.h"
#include "FullscreenQuad.h"
#include "DeviceStates.h"

struct CB_DIRECTIONALLIGHT_CAMERA_PROPERTIES
{
	D3DXMATRIX InverseViewProjection;
	D3DXVECTOR4 CameraPosition;
};

struct CB_DIRECTIONALLIGHT_PROPERTIES
{
	D3DXVECTOR3 LightDirection;
	float LightIntensity;
	D3DXCOLOR LightColor;
};

class DirectionalLightRenderer : public LightRenderer<DirectionalLight>
{
private:
	ID3D11PixelShader* _unshadowedPS;
	ID3D11PixelShader* _shadowedPS;
	ID3D11Buffer* _cameraPropertiesBuffer;
	ID3D11Buffer* _lightPropertiesBuffer;
	FullscreenQuad _fsQuad;

	static const UINT NUM_SHADOW_MAPS = 3;
	static const UINT NUM_CASCADES = 4;
	static const UINT SHADOW_MAP_SIZE = 4096;
	ID3D11Texture2D* _shadowMapTextures[NUM_SHADOW_MAPS];
	ID3D11RenderTargetView* _shadowMapRTVs[NUM_SHADOW_MAPS];
	ID3D11ShaderResourceView* _shadowMapSRVs[NUM_SHADOW_MAPS];
	ID3D11Texture2D* _shadowMapDSTexture;
	ID3D11DepthStencilView* _shadowMapDSView;

public:
	DirectionalLightRenderer();

	HRESULT RenderShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, std::vector<ModelInstance*> models,
		Camera* camera, BoundingBox* sceneBounds);
	HRESULT RenderLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
		GBuffer* gBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};