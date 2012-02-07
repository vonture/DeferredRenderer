#include "PCH.h"
#include "ParticleRenderer.h"

#include "VertexShaderLoader.h"
#include "GeometryShaderLoader.h"
#include "PixelShaderLoader.h"

ParticleRenderer::ParticleRenderer()
	: _ps(NULL), _gs(NULL), _il(NULL), _vs(NULL), _particleCB(NULL), _cameraCB(NULL), _particleBlend(NULL)
{
	SetFadeDistance(0.5f);
}

HRESULT ParticleRenderer::RenderParticles(ID3D11DeviceContext* pd3dDeviceContext,
	vector<ParticleSystemInstance*>* instances,  Camera* camera, GBuffer* gBuffer)
{
	UINT partCount = instances->size();
	if (partCount > 0)
	{
		HRESULT hr;

		// Sort systems by the emitter's depth from the camera
		XMFLOAT3 camForward = camera->GetForward();
		std::vector<PARTICLE_SYSTEM_INFO> depthVec = std::vector<PARTICLE_SYSTEM_INFO>(partCount);
		for (UINT i = 0; i < partCount; i++)
		{
			XMFLOAT3 systemPos = instances->at(i)->GetPosition();
			float depth = systemPos.x * camForward.x + systemPos.y * camForward.y + systemPos.z * camForward.z;
			depthVec[i] = PARTICLE_SYSTEM_INFO(i, depth);
		}

		std::sort(depthVec.begin(), depthVec.end(), depthCompare);

		pd3dDeviceContext->VSSetShader(_vs, NULL, 0);
		pd3dDeviceContext->GSSetShader(_gs, NULL, 0);
		pd3dDeviceContext->PSSetShader(_ps, NULL, 0);

		pd3dDeviceContext->IASetInputLayout(_il);

		float blendFactor[4] = {1, 1, 1, 1};
		pd3dDeviceContext->OMSetBlendState(_particleBlend, blendFactor, 0xFFFFFFFF);
		
		ID3D11SamplerState* samplers[2] = 
		{
			_samplerStates.GetLinearClamp(),
			_samplerStates.GetPointClamp(),
		};
		pd3dDeviceContext->PSSetSamplers(0, 2, samplers);

		pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		pd3dDeviceContext->OMSetDepthStencilState(_dsStates.GetDepthEnabled(), 0);

		XMFLOAT4X4 fViewProj = camera->GetViewProjection();
		XMMATRIX viewProj = XMLoadFloat4x4(&fViewProj);

		XMFLOAT4X4 fInvView = camera->GetWorld();
		XMMATRIX invView = XMLoadFloat4x4(&fInvView);

		XMFLOAT4X4 fPrevViewProj = camera->GetPreviousViewProjection();
		XMMATRIX prevViewProj = XMLoadFloat4x4(&fPrevViewProj);

		XMFLOAT4X4 fPrevInvView = camera->GetPreviousWorld();
		XMMATRIX prevInvView = XMLoadFloat4x4(&fPrevInvView);
						
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		// Map Geometry shader perameters
		V_RETURN(pd3dDeviceContext->Map(_particleCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_PARTICLE_PROPERTIES* particleProperties = (CB_PARTICLE_PROPERTIES*)mappedResource.pData;

		XMStoreFloat4x4(&particleProperties->ViewProjection, XMMatrixTranspose(viewProj));
		XMStoreFloat4x4(&particleProperties->InverseView, XMMatrixTranspose(invView));
		XMStoreFloat4x4(&particleProperties->PrevViewProjection, XMMatrixTranspose(prevViewProj));
		XMStoreFloat4x4(&particleProperties->PrevInverseView, XMMatrixTranspose(prevInvView));

		pd3dDeviceContext->Unmap(_particleCB, 0);
		pd3dDeviceContext->GSSetConstantBuffers(0, 1, &_particleCB);

		// Map pixel shader parameters
		V_RETURN(pd3dDeviceContext->Map(_cameraCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_CAMERA_PROPERTIES* cameraProperties = (CB_CAMERA_PROPERTIES*)mappedResource.pData;

		cameraProperties->CameraNearClip = camera->GetNearClip();
		cameraProperties->CameraFarClip = camera->GetFarClip();
		cameraProperties->FadeDistance = _fadeDistance;
		cameraProperties->Forward = camera->GetForward();
		cameraProperties->Up = camera->GetUp();
		cameraProperties->Right = camera->GetRight();

		pd3dDeviceContext->Unmap(_cameraCB, 0);
		pd3dDeviceContext->PSSetConstantBuffers(0, 1, &_cameraCB);

		ID3D11ShaderResourceView* depthSRV = gBuffer->GetDepthSRV();
		pd3dDeviceContext->PSSetShaderResources(2, 1, &depthSRV);

		for (UINT i = 0; i < partCount; i++)
		{
			ParticleSystemInstance* system = instances->at(depthVec[i].System);
						
			ID3D11ShaderResourceView* srvs[2] = { system->GetDiffuseSRV(), system->GetNormalSRV() };
			pd3dDeviceContext->PSSetShaderResources(0, 2, srvs);

			ID3D11Buffer* vertexBuffers[1] = { system->GetParticleVertexBuffer(pd3dDeviceContext, camera) };
			UINT strides[1] = { system->GetVertexStride() };
			UINT offsets[1] = { 0 };
			pd3dDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);

			pd3dDeviceContext->Draw(system->GetParticleCount(), 0);
		}

		ID3D11ShaderResourceView* nullSrvs[2] = { NULL, NULL };
		pd3dDeviceContext->PSSetShaderResources(0, 2, nullSrvs);

		ID3D11Buffer* nullBuf = NULL;
		pd3dDeviceContext->GSSetConstantBuffers(0, 1, &nullBuf);
		pd3dDeviceContext->PSSetConstantBuffers(1, 1, &nullBuf);

		pd3dDeviceContext->VSSetShader(NULL, NULL, 0);
		pd3dDeviceContext->GSSetShader(NULL, NULL, 0);
		pd3dDeviceContext->PSSetShader(NULL, NULL, 0);
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
		{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "PREVPOSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",			0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "RADIUS",			0, DXGI_FORMAT_R32_FLOAT,			0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "PREVRADIUS",		0, DXGI_FORMAT_R32_FLOAT,			0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "ROTATION",		0, DXGI_FORMAT_R32_FLOAT,			0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "PREVROTATION",	0, DXGI_FORMAT_R32_FLOAT,			0, 52, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
		0, //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	bufferDesc.ByteWidth = sizeof(CB_PARTICLE_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_particleCB));
	V_RETURN(SetDXDebugName(_particleCB, "Particle Renderer particle CB"));

	bufferDesc.ByteWidth = sizeof(CB_CAMERA_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_cameraCB));
	V_RETURN(SetDXDebugName(_particleCB, "Particle Renderer camera CB"));
		
	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = true;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	for (UINT i = 1; i < 8; i++)
	{
		blendDesc.RenderTarget[i].BlendEnable = true;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
	}
	V_RETURN(pd3dDevice->CreateBlendState(&blendDesc, &_particleBlend));
	
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

	SAFE_RELEASE(_particleCB);
	SAFE_RELEASE(_cameraCB);

	SAFE_RELEASE(_particleBlend);

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
