#pragma once

#include "PCH.h"
#include "Lights.h"
#include "LightRenderer.h"
#include "ModelInstance.h"

class SpotLightRenderer : public LightRenderer<SpotLight>
{
private:
    static const UINT NUM_SHADOW_MAPS = 3;

protected:
    UINT GetMaxShadowedLights() const { return NUM_SHADOW_MAPS; }

public:
    SpotLightRenderer();
    ~SpotLightRenderer();

    HRESULT RenderGeometryShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, std::vector<ModelInstance*>* models,
        Camera* camera, AxisAlignedBox* sceneBounds);
    HRESULT RenderGeometryLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera, GBuffer* gBuffer);

    HRESULT RenderParticleLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
        ParticleBuffer* gBuffer);

    HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11DestroyDevice(ContentManager* pContentManager);

    HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};