#pragma once

#include "PCH.h"
#include "DeviceStates.h"
#include "IHasContent.h"
#include "Particle.h"
#include "ParticleSystem.h"
#include "ParticleSystemInstance.h"
#include "GBuffer.h"
#include "Camera.h"

#include "VertexShaderLoader.h"
#include "GeometryShaderLoader.h"
#include "PixelShaderLoader.h"

class ParticleRenderer : public IHasContent
{
private:
    PixelShaderContent* _ps;
    GeometryShaderContent* _gs;
    VertexShaderContent* _vs;

    float _fadeDistance;

    DepthStencilStates _dsStates;
    SamplerStates _samplerStates;
    BlendStates _blendStates;
    RasterizerStates _rasterStates;

    ID3D11BlendState* _particleBlend;

    ID3D11Buffer* _particleCB;
    struct CB_PARTICLE_PROPERTIES
    {
        XMFLOAT4X4 ViewProjection;
        XMFLOAT4X4 InverseView;
        XMFLOAT4X4 PrevViewProjection;
        XMFLOAT4X4 PrevInverseView;
    };

    ID3D11Buffer* _cameraCB;
    struct CB_CAMERA_PROPERTIES
    {
        float CameraNearClip;
        float CameraFarClip;
        float FadeDistance;
        float Padding0;
        XMFLOAT3 Forward;
        float Padding1;
        XMFLOAT3 Up;
        float Padding2;
        XMFLOAT3 Right;
        float Padding3;
    };

    struct PARTICLE_SYSTEM_INFO
    {
        UINT System;
        float Depth;

        PARTICLE_SYSTEM_INFO() { }
        PARTICLE_SYSTEM_INFO(UINT system, float depth) : System(system), Depth(depth) { }
    };
    static bool depthCompare(const PARTICLE_SYSTEM_INFO& i, const PARTICLE_SYSTEM_INFO& j)
    {
        return i.Depth > j.Depth;
    }

public:
    ParticleRenderer();

    float GetFadeDistance() const { return _fadeDistance; }
    void SetFadeDistance(float val) { _fadeDistance = val; }

    HRESULT RenderParticles(ID3D11DeviceContext* pd3dDeviceContext, vector<ParticleSystemInstance*>* instances,
        Camera* camera, GBuffer* gBuffer);

    HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11DestroyDevice(ContentManager* pContentManager);

    HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
        IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
    void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};
