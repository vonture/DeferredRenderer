#include "PCH.h"
#include "ParticleRenderer.h"

#include "VertexShaderLoader.h"
#include "GeometryShaderLoader.h"
#include "PixelShaderLoader.h"

ParticleRenderer::ParticleRenderer()
	: _ps(NULL), _gs(NULL), _il(NULL), _vs(NULL), _constantBuffer(NULL)
{

}

HRESULT ParticleRenderer::RenderParticles(ID3D11DeviceContext* pd3dDeviceContext,
	vector<ParticleSystemInstance*>* instances, Camera* camera)
{
	UINT partCount = instances->size();
	if (partCount > 0)
	{
		BEGIN_EVENT_D3D(L"Particle Rendering");

		HRESULT hr;


		pd3dDeviceContext->VSSetShader(_vs, NULL, 0);
		pd3dDeviceContext->GSSetShader(_gs, NULL, 0);
		pd3dDeviceContext->PSSetShader(_ps, NULL, 0);

		pd3dDeviceContext->IASetInputLayout(_il);

		float blendFactor[4] = {1, 1, 1, 1};
		pd3dDeviceContext->OMSetBlendState(_blendStates.GetAdditiveBlend(), blendFactor, 0xFFFFFFFF);
		
		ID3D11SamplerState* sampler = _samplerStates.GetLinearClamp();
		pd3dDeviceContext->PSSetSamplers(0, 1, &sampler);

		pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		pd3dDeviceContext->OMSetDepthStencilState(_dsStates.GetDepthEnabled(), 0);


		XMFLOAT4X4 fViewProj = camera->GetViewProjection();
		XMMATRIX viewProj = XMLoadFloat4x4(&fViewProj);

		XMFLOAT4X4 fInvView = camera->GetWorld();
		XMMATRIX invView = XMLoadFloat4x4(&fInvView);

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		V_RETURN(pd3dDeviceContext->Map(_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_PARTICLE_PROPERTIES* properties = (CB_PARTICLE_PROPERTIES*)mappedResource.pData;

		XMStoreFloat4x4(&properties->ViewProjection, XMMatrixTranspose(viewProj));
		XMStoreFloat4x4(&properties->InverseView, XMMatrixTranspose(invView));

		pd3dDeviceContext->Unmap(_constantBuffer, 0);
		pd3dDeviceContext->GSSetConstantBuffers(0, 1, &_constantBuffer);

		for (UINT i = 0; i < partCount; i++)
		{
			ParticleSystemInstance* system = instances->at(i);
						
			ID3D11ShaderResourceView* srvs[2] = { system->GetDiffuseSRV(), system->GetNormalSRV() };
			pd3dDeviceContext->PSSetShaderResources(0, 2, srvs);

			ID3D11Buffer* vertexBuffers[1] = { system->GetParticleVertexBuffer(pd3dDeviceContext) };
			UINT strides[1] = { system->GetVertexStride() };
			UINT offsets[1] = { 0 };
			pd3dDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);

			pd3dDeviceContext->Draw(system->GetParticleCount(), 0);
		}

		ID3D11ShaderResourceView* nullSrvs[2] = { NULL, NULL };
		pd3dDeviceContext->PSGetShaderResources(0, 2, nullSrvs);

		ID3D11Buffer* nullBuf = NULL;
		pd3dDeviceContext->GSSetConstantBuffers(0, 1, &nullBuf);

		pd3dDeviceContext->VSSetShader(NULL, NULL, 0);
		pd3dDeviceContext->GSSetShader(NULL, NULL, 0);
		pd3dDeviceContext->PSSetShader(NULL, NULL, 0);

		END_EVENT_D3D(L"");
	}
	return S_OK;
}

HRESULT ParticleRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	
	// Load pixel shader
	PixelShaderContent* psContent;
	PixelShaderOptions psOpts =
	{
		"PS_Particle",	// const char* EntryPoint;
		NULL,			// D3D_SHADER_MACRO* Defines;
		"Particle",		// const char* DebugName;
	};
	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"Particle.hlsl", &psOpts, &psContent));

	_ps = psContent->PixelShader;
	_ps->AddRef();

	SAFE_RELEASE(psContent);

	// Load geometry shader
	GeometryShaderContent* gsContent;
	GeometryShaderOptions gsOpts =
	{
		"GS_Particle",	// const char* EntryPoint;
		NULL,			// D3D_SHADER_MACRO* Defines;
		"Particle",		// const char* DebugName;
	};
	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"Particle.hlsl", &gsOpts, &gsContent));

	_gs = gsContent->GeometryShader;
	_gs->AddRef();

	SAFE_RELEASE(gsContent);

	// Load vertex shader (using ParticleVertex)
	D3D11_INPUT_ELEMENT_DESC layout_particle[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,	  0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "RADIUS",    0, DXGI_FORMAT_R32_FLOAT,          0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "ROTATION",  0, DXGI_FORMAT_R32_FLOAT,          0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	VertexShaderContent* vsContent;
	VertexShaderOptions vsOpts =
	{
		"VS_Particle",				// const char* EntryPoint;
		NULL,						// D3D_SHADER_MACRO* Defines;
		layout_particle,			// D3D11_INPUT_ELEMENT_DESC* InputElements;
		ARRAYSIZE(layout_particle),	// UINT InputElementCount;
		"Particle"					// const char* DebugName;
	};

	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"Particle.hlsl", &vsOpts, &vsContent));

	_vs = vsContent->VertexShader;
	_il = vsContent->InputLayout;

	_vs->AddRef();
	_il->AddRef();

	SAFE_RELEASE(vsContent);

	D3D11_BUFFER_DESC bufferDesc =
	{
		sizeof(CB_PARTICLE_PROPERTIES), //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_constantBuffer));
	V_RETURN(SetDXDebugName(_constantBuffer, "Particle Renderer CB"));

	V_RETURN(_dsStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	V_RETURN(_samplerStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	V_RETURN(_blendStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	V_RETURN(_rasterStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	
	return S_OK;
}

void ParticleRenderer::OnD3D11DestroyDevice()
{
	SAFE_RELEASE(_ps);	
	SAFE_RELEASE(_gs);

	SAFE_RELEASE(_il);
	SAFE_RELEASE(_vs);

	SAFE_RELEASE(_constantBuffer);

	_dsStates.OnD3D11DestroyDevice();
	_samplerStates.OnD3D11DestroyDevice();
	_blendStates.OnD3D11DestroyDevice();
	_rasterStates.OnD3D11DestroyDevice();
}

HRESULT ParticleRenderer::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager,
	IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(_dsStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_samplerStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_blendStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_rasterStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void ParticleRenderer::OnD3D11ReleasingSwapChain()
{
	_dsStates.OnD3D11ReleasingSwapChain();
	_samplerStates.OnD3D11ReleasingSwapChain();
	_blendStates.OnD3D11ReleasingSwapChain();
	_rasterStates.OnD3D11ReleasingSwapChain();
}