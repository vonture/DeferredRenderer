#pragma once

#include "PCH.h"
#include "IHasContent.h"

class LightBuffer : public IHasContent
{
private:
    // LightRT0 =  Geometry Light.r  | Geometry Light.g   | Geometry Light.b   | Geometry Specular
    // LightRT1 =  Particle Light.r  | Particle Light.g   | Particle Light.b   |
    ID3D11ShaderResourceView* _srv[2];
    ID3D11RenderTargetView* _rtv[2];

    XMFLOAT3 _ambientColor;
    float _ambientBrightness;

public:
    LightBuffer();

    const XMFLOAT3& GetAmbientColor() const { return _ambientColor; }
    void SetAmbientColor(const XMFLOAT3& color) { _ambientColor = color; }

    float GetAmbientBrightness() const { return _ambientBrightness; }
    void SetAmbientBrightness(float brightness) { _ambientBrightness = brightness; }

    ID3D11ShaderResourceView* GetGeometryLightSRV() { return _srv[0]; }
    ID3D11RenderTargetView* GetGeometryLightRTV() { return _rtv[0]; }

    ID3D11ShaderResourceView* GetParticleLightSRV() { return _srv[1]; }
    ID3D11RenderTargetView* GetParticleLightRTV() { return _rtv[1]; }

    HRESULT Clear(ID3D11DeviceContext* pd3dImmediateContext);

    HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11DestroyDevice(ContentManager* pContentManager);

    HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};