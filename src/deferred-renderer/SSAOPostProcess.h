#pragma once

#include "PCH.h"
#include "PostProcess.h"
#include "PixelShaderLoader.h"

class SSAOPostProcess : public PostProcess
{
private:
    float _sampleRadius;
    float _blurSigma;
    float _depthThreshold;
    bool _halfRes;
    UINT _sampleCountIndex;

    XMFLOAT2 _invSceneSize;

#ifdef ALL_PRESETS
    static const UINT NUM_SSAO_SAMPLE_COUNTS = 7;
    static const UINT SSAO_SAMPLE_COUNT_MAX = 256;
#else
    static const UINT NUM_SSAO_SAMPLE_COUNTS = 2;
    static const UINT SSAO_SAMPLE_COUNT_MAX = 64;
#endif
    static const UINT SSAO_SAMPLE_COUNTS[NUM_SSAO_SAMPLE_COUNTS];

    XMFLOAT4 _sampleDirections[SSAO_SAMPLE_COUNT_MAX];

    ID3D11RenderTargetView* _aoRTV;
    ID3D11ShaderResourceView* _aoSRV;

    ID3D11RenderTargetView* _blurTempRTV;
    ID3D11ShaderResourceView* _blurTempSRV;

    static const UINT RANDOM_TEXTURE_SIZE = 32;
    ID3D11ShaderResourceView* _randomSRV;

    PixelShaderContent* _aoPSs[NUM_SSAO_SAMPLE_COUNTS];
    PixelShaderContent* _hBlurPS[2];
    PixelShaderContent* _vBlurPS[2];
    PixelShaderContent* _compositePS[2];

    ID3D11Buffer* _aoPropertiesBuffer;
    ID3D11Buffer* _sampleDirectionsBuffers[NUM_SSAO_SAMPLE_COUNTS];

    struct CB_AO_PROPERTIES
    {
        XMFLOAT4X4 ViewProjection;
        XMFLOAT4X4 InverseViewProjection;
        float SampleRadius;
        float BlurSigma;
        float GaussianNumerator;
        float CameraNearClip;
        float CameraFarClip;
        float DepthThreshold;
        XMFLOAT2 InverseSceneSize;
    };

public:
    SSAOPostProcess();

    float GetSampleRadius() const { return _sampleRadius; }
    void SetSampleRadius(float radius) { _sampleRadius = max(radius, 0.0f); }

    float GetBlurSigma() const { return _blurSigma; }
    void SetBlurSigma(float sigma) { _blurSigma = max(sigma, 0.1f); }

    float GetDepthThreshold() const { return _depthThreshold; }
    void SetDepthTreshold(float thresh) { _depthThreshold = max(thresh, EPSILON); }

    bool GetHalfResolution() const { return _halfRes; }
    void SetHalfResolution(bool val) { _halfRes = val; }

    UINT GetSampleCount() const { return SSAO_SAMPLE_COUNTS[_sampleCountIndex]; }

    UINT GetSampleCountIndex() const { return _sampleCountIndex; }
    void SetSampleCountIndex(UINT index) { _sampleCountIndex = min(index, NUM_SSAO_SAMPLE_COUNTS - 1); }
    UINT GetNumSampleCountIndices() const { return NUM_SSAO_SAMPLE_COUNTS; }

    HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
        ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, ParticleBuffer* pBuffer,LightBuffer* lightBuffer);

    HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11DestroyDevice(ContentManager* pContentManager);

    HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};