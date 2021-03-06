#pragma once

#include "PCH.h"
#include "IHasContent.h"
#include "DeviceStates.h"
#include "ModelInstance.h"
#include "Camera.h"
#include "GBuffer.h"
#include "ParticleBuffer.h"

class LightRendererBase : public IHasContent
{
private:
    DepthStencilStates _dsStates;
    SamplerStates _samplerStates;
    BlendStates _blendStates;
    RasterizerStates _rasterStates;

    bool _alphaCutoutEnabled;
    float _alphaThreshold;

protected:
    DepthStencilStates* GetDepthStencilStates() { return &_dsStates; }
    SamplerStates* GetSamplerStates() { return &_samplerStates; }
    BlendStates* GetBlendStates() { return &_blendStates; }
    RasterizerStates* GetRasterizerStates() { return &_rasterStates; }

    virtual UINT GetMaxShadowedLights() const = 0;

public:
    LightRendererBase();
    virtual ~LightRendererBase();

    virtual HRESULT RenderGeometryShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, std::vector<ModelInstance*>* models,
        Camera* camera, AxisAlignedBox* sceneBounds) = 0;
    virtual HRESULT RenderGeometryLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
        GBuffer* gBuffer) = 0;

    virtual HRESULT RenderParticleLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
        ParticleBuffer* gBuffer) = 0;

    virtual UINT GetCount() = 0;
    virtual UINT GetCount(bool shadowed) = 0;

    virtual void Clear() = 0;

    bool GetAlphaCutoutEnabled() const { return _alphaCutoutEnabled; }
    void SetAlphaCutoutEnabled(bool enabled) { _alphaCutoutEnabled = enabled; }

    float GetAlphaThreshold() const { return _alphaThreshold; }
    void SetAlphaThreshold(float threshold) { _alphaThreshold = clamp(threshold, 0.0f, 1.0f); }

    virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    virtual void OnD3D11DestroyDevice(ContentManager* pContentManager);

    virtual HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    virtual void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};