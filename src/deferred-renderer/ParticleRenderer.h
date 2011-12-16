#pragma once

#include "PCH.h"
#include "DeviceStates.h"
#include "IHasContent.h"
#include "Particle.h"
#include "ParticleSystem.h"
#include "ParticleSystemInstance.h"
#include "Camera.h"

class ParticleRenderer : public IHasContent
{
private:
	ID3D11PixelShader* _ps;
	ID3D11GeometryShader* _gs;

	ID3D11InputLayout* _il;
	ID3D11VertexShader* _vs;

	DepthStencilStates _dsStates;
	SamplerStates _samplerStates;
	BlendStates _blendStates;
	RasterizerStates _rasterStates;

	ID3D11Buffer* _constantBuffer;
	struct CB_PARTICLE_PROPERTIES
	{
		XMFLOAT4X4 ViewProjection;
		XMFLOAT4X4 InverseView;
	};

public:
	ParticleRenderer();

	HRESULT RenderParticles(ID3D11DeviceContext* pd3dDeviceContext, vector<ParticleSystemInstance*>* instances,
		Camera* camera);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, 
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, 
		IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};
