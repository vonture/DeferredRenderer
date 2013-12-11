#pragma once

#include "PCH.h"
#include "PostProcess.h"
#include "PixelShaderLoader.h"

class FilmGrainVignettePostProcess : public PostProcess
{
private:
    PixelShaderContent* _ps[2][2];

    struct CB_PROPERTIES
    {
        float DeltaTime;
        float FilmGrainStrength;
        float VignetteStart;
        float VignetteStrength;
    };
    ID3D11Buffer* _propertiesBuffer;

    bool _filmGrainEnabled;
    float _filmGrainStrength;
    double _totalTime;

    bool _vignetteEnabled;
    float _vignetteStart;
    float _vignetteStrength;

public:
    FilmGrainVignettePostProcess();

    bool GetFilmGrainEnabled() const { return _filmGrainEnabled; }
    void SetFilmGrainEnabled(bool val) { _filmGrainEnabled = val; }

    float GetFilmGrainStrength() const { return _filmGrainStrength; }
    void SetFilmGrainStrength(float val) { _filmGrainStrength = clamp(val, 0.0f, 25.0f); }

    double GetTime() const { return _totalTime; }
    void SetTime(double val) { _totalTime = max(val, 0.0f); }

    bool GetVignetteEnabled() const { return _vignetteEnabled; }
    void SetVignetteEnabled(bool val) { _vignetteEnabled = val; }

    float GetVignetteStart() const { return _vignetteStart; }
    void SetVignetteStart(float val) { _vignetteStart = max(val, 0.0f); }

    float GetVignetteStrength() const { return _vignetteStrength; }
    void SetVignetteStrength(float val) { _vignetteStrength = max(val, 0.0f); }

    HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
        ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, ParticleBuffer* pBuffer,
        LightBuffer* lightBuffer);

    HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11DestroyDevice(ContentManager* pContentManager);

    HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
        IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};