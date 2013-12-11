#pragma once

#include "PCH.h"
#include "PostProcess.h"
#include "PixelShaderLoader.h"
#include "TextureLoader.h"

class HDRPostProcess : public PostProcess
{
private:
    float _timeDelta;
    float _tau;
    float _lumWhite;
    float _exposureKey;
    float _bloomThreshold;
    float _bloomMagnitude;
    float _bloomBlurSigma;

    UINT _lumMapSize;
    UINT _mipLevels;

    ID3D11RenderTargetView* _lumRTVs[2];
    ID3D11ShaderResourceView* _lumSRVs[2];

    ID3D11RenderTargetView* _downScaleRTVs[3];
    ID3D11ShaderResourceView* _downScaleSRVs[3];

    ID3D11RenderTargetView* _blurTempRTV;
    ID3D11ShaderResourceView* _blurTempSRV;

    TextureContent* _colorGradeSRV;

    PixelShaderContent* _luminanceMapPS;
    PixelShaderContent* _toneMapPS;
    PixelShaderContent* _scalePS;
    PixelShaderContent* _thresholdPS;
    PixelShaderContent* _hBlurPS;
    PixelShaderContent* _vBlurPS;

    ID3D11Buffer* _hdrPropertiesBuffer;

    XMFLOAT2 _invSceneSize;

    struct CB_HDR_PROPERTIES
    {
        float Tau;
        float WhiteLuminancePercentage;
        float ExposureKey;
        float TimeDelta;
        UINT MipLevels;
        float BloomThreshold;
        float BloomMagnitude;
        float BloomBlurSigma;
        float GaussianNumerator;
        XMFLOAT2 InverseSceneSize;
        float Padding;
    };

    void swapLuminanceBuffers();

public:
    HDRPostProcess();

    float GetTimeDelta() const { return _timeDelta; }
    void SetTimeDelta(float dt) { _timeDelta = max(dt, 0.0f); }

    float GetTau() const { return _tau; }
    void SetTau(float tau) { _tau = saturate(tau); }

    float GetWhiteLuminancePercentage() const { return _lumWhite; }
    void SetWhiteLuminancePercentage(float lumWhite) { _lumWhite = max(lumWhite, 0.0f); }

    float GetBloomThreshold() const { return _bloomThreshold; }
    void SetBloomThreshold(float thresh) { _bloomThreshold = saturate(thresh); }

    float GetBloomMagnitude() const { return _bloomMagnitude; }
    void SetBloomMagnitude(float mag) { _bloomMagnitude = max(mag, 0.0f); }

    float GetBloomBlurSigma() const { return _bloomBlurSigma; }
    void SetBloomBlurSigma(float sigma) { _bloomBlurSigma = max(sigma, 0.0f); }

    float GetExposureKey() const { return _exposureKey; }
    void SetExposureKey(float key) { _exposureKey = clamp(key, 0.0f, 1.0f); }

    HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
        ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, ParticleBuffer* pBuffer,LightBuffer* lightBuffer);

    HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11DestroyDevice(ContentManager* pContentManager);

    HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};