#include "PCH.h"
#include "ModelRenderer.h"
#include "PixelShaderLoader.h"
#include "VertexShaderLoader.h"

ModelRenderer::ModelRenderer()
	: _meshVertexShader(NULL),  _meshInputLayout(NULL), _alphaThresholdBuffer(NULL), _constantBuffer(NULL)
{
	for (UINT i = 0; i < 2; i++)
	{
		for (UINT j = 0; j < 2; j++)
		{
			for (UINT k = 0; k < 2; k++)
			{
				_meshPixelShader[i][j][k][0] = NULL;
				_meshPixelShader[i][j][k][1] = NULL;
			}
		}		
	}

	SetAlphaCutoutEnabled(true);
	SetAlphaThreshold(0.05f);
}


HRESULT ModelRenderer::RenderModels(ID3D11DeviceContext* pd3dDeviceContext,
	vector<ModelInstance*>* instances, Camera* camera)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;	

	XMFLOAT4X4 fViewProj = camera->GetViewProjection();
	XMMATRIX viewProj = XMLoadFloat4x4(&fViewProj);

	XMFLOAT4X4 fProj = camera->GetProjection();
	XMMATRIX proj = XMLoadFloat4x4(&fProj);

	Frustum cameraFrust = camera->CreateFrustum();

	pd3dDeviceContext->VSSetShader(_meshVertexShader, NULL, 0);

	pd3dDeviceContext->IASetInputLayout(_meshInputLayout);
	pd3dDeviceContext->OMSetDepthStencilState(_dsStates.GetDepthWriteStencilSetDesc(), 1);

	pd3dDeviceContext->RSSetState(_rasterStates.GetBackFaceCull());

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dDeviceContext->OMSetBlendState(_blendStates.GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

	ID3D11SamplerState* sampler = _samplerStates.GetAnisotropic16Wrap();
	pd3dDeviceContext->PSSetSamplers(0, 1, &sampler);

	// if Alpha cutout is enabled, map the alpha cutout info buffer
	if (_alphaCutoutEnabled)
	{
		V_RETURN(pd3dDeviceContext->Map(_alphaThresholdBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_MODEL_ALPHA_THRESHOLD* cutoutProperties = (CB_MODEL_ALPHA_THRESHOLD*)mappedResource.pData;
		cutoutProperties->AlphaThreshold = _alphaThreshold;
		pd3dDeviceContext->Unmap(_alphaThresholdBuffer, 0);

		pd3dDeviceContext->PSSetConstantBuffers(1, 1, &_alphaThresholdBuffer);
	}

	ID3D11PixelShader* prevPS = NULL;

	for (UINT i = 0; i < instances->size(); i++)
	{
		ModelInstance* instance = instances->at(i);
		Model* model = instance->GetModel();

		OrientedBox modelBounds = instance->GetOrientedBox();

		// If there is no intersection between the frust and this model, don't render anything
		int modelIntersect = Collision::IntersectOrientedBoxFrustum(&modelBounds, &cameraFrust);
		if (!modelIntersect)
		{
			continue;
		}
		
		XMFLOAT4X4 fWorld = instance->GetWorld();
		XMMATRIX world = XMLoadFloat4x4(&fWorld);

		XMMATRIX wvp = XMMatrixMultiply(world, viewProj);

		V(pd3dDeviceContext->Map(_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_MODEL_PROPERTIES* modelProperties = (CB_MODEL_PROPERTIES*)mappedResource.pData;
		XMStoreFloat4x4(&modelProperties->World, XMMatrixTranspose(world));
		XMStoreFloat4x4(&modelProperties->WorldViewProjection, XMMatrixTranspose(wvp));
		pd3dDeviceContext->Unmap(_constantBuffer, 0);

		pd3dDeviceContext->VSSetConstantBuffers(0, 1, &_constantBuffer);

		for (UINT j = 0; j < model->GetMeshCount(); j++)
		{
			// If the main box is completely within the frust, we can skip the mesh check
			if (modelIntersect != COMPLETELY_INSIDE)
			{
				const OrientedBox& meshBounds = instance->GetMeshOrientedBox(j);
							
				if (!Collision::IntersectOrientedBoxFrustum(&meshBounds, &cameraFrust))
				{
					continue;
				}
			}

			const Mesh* mesh = model->GetMesh(j);
			UINT partCount = mesh->GetMeshPartCount();

			ID3D11Buffer* vertexBuffers[1] = { mesh->GetVertexBuffer() };
			UINT strides[1] = { mesh->GetVertexStride() };
			UINT offsets[1] = { 0 };

			pd3dDeviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
			pd3dDeviceContext->IASetIndexBuffer(mesh->GetIndexBuffer(), mesh->GetIndexBufferFormat(), 0);
			pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			for (UINT k = 0; k < partCount; k++)
			{
				const MeshPart* part = mesh->GetMeshPart(k);
				const Material* mat = model->GetMaterial(part->MaterialIndex);

				ID3D11Buffer* buf = mat->GetPropertiesBuffer();
				pd3dDeviceContext->PSSetConstantBuffers(0, 1, &buf);

				ID3D11ShaderResourceView* diffSRV = mat->GetDiffuseSRV();
				ID3D11ShaderResourceView* normSRV = mat->GetNormalSRV();
				ID3D11ShaderResourceView* specSRV = mat->GetSpecularSRV();

				ID3D11ShaderResourceView* srvs[3] = { diffSRV, normSRV, specSRV };
				pd3dDeviceContext->PSSetShaderResources(0, 3, srvs);
				
				// Set the shader if it wasn't the same for the last mesh
				ID3D11PixelShader* ps = _meshPixelShader[diffSRV != NULL]
					[normSRV != NULL][specSRV != NULL][_alphaCutoutEnabled && mesh->GetAlphaCutoutEnabled()];
				if (ps != prevPS)
				{
					pd3dDeviceContext->PSSetShader(ps, NULL, 0);
					prevPS = ps;
				}

				pd3dDeviceContext->DrawIndexed(part->IndexCount, part->IndexStart, part->VertexStart);
			}
		}
	}

	// Unset the alpha cutout buffer
	if (_alphaCutoutEnabled)
	{
		ID3D11Buffer* nullBuffer[1] = { NULL };
		pd3dDeviceContext->PSSetConstantBuffers(2, 1, nullBuffer);
	}

	return S_OK;
}

HRESULT ModelRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	PixelShaderContent* psContent;
	VertexShaderContent* vsContent;

	D3D_SHADER_MACRO meshMacros[] = 
	{		
		{ "DIFFUSE_MAPPED", "" },
		{ "NORMAL_MAPPED", "" },
		{ "SPECULAR_MAPPED", "" },
		{ "ALPHA_CUTOUT_ENABLED", "" },
		NULL,
	};

	char debugName[256];

	PixelShaderOptions psOpts =
	{
		"PS_Mesh",	// const char* EntryPoint;
		meshMacros,	// D3D_SHADER_MACRO* Defines;
		debugName,	// const char* DebugName;
	};
	
	for (UINT i = 0; i < 2; i++)
	{
		meshMacros[0].Definition = i ? "1" : "0";

		for (UINT j = 0; j < 2; j++)
		{
			meshMacros[1].Definition = j ? "1" : "0";

			for (UINT k = 0; k < 2; k++)
			{
				meshMacros[2].Definition = k ? "1" : "0";

				// Load no alpha cutout
				meshMacros[3].Definition = "0";
				sprintf_s(debugName, "G-Buffer Mesh (diffuse = %u, normal = %u, specular = %u, alpha cutout = %u)",
					i, j, k, 0);
				V_RETURN(pContentManager->LoadContent(pd3dDevice, L"Mesh.hlsl", &psOpts, &psContent));

				_meshPixelShader[i][j][k][0] = psContent->PixelShader;
				_meshPixelShader[i][j][k][0]->AddRef();

				SAFE_RELEASE(psContent);

				// Load alpha cutout
				meshMacros[3].Definition = "1";
				sprintf_s(debugName, "G-Buffer Mesh (diffuse = %u, normal = %u, specular = %u, alpha cutout = %u)",
					i, j, k, 1);
				V_RETURN(pContentManager->LoadContent(pd3dDevice, L"Mesh.hlsl", &psOpts, &psContent));

				_meshPixelShader[i][j][k][1] = psContent->PixelShader;
				_meshPixelShader[i][j][k][1]->AddRef();

				SAFE_RELEASE(psContent);
			}
		}
	}

	// Load the vertex shader
	D3D11_INPUT_ELEMENT_DESC layout_mesh[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	VertexShaderOptions vsOpts =
	{
		"VS_Mesh",				// const char* EntryPoint;
		NULL,					// D3D_SHADER_MACRO* Defines;
		layout_mesh,			// D3D11_INPUT_ELEMENT_DESC* InputElements;
		ARRAYSIZE(layout_mesh),	// UINT InputElementCount;
		"G-Buffer Mesh"			// const char* DebugName;
	};

	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"Mesh.hlsl", &vsOpts, &vsContent));

	_meshVertexShader = vsContent->VertexShader;
	_meshInputLayout = vsContent->InputLayout;

	_meshVertexShader->AddRef();
	_meshInputLayout->AddRef();

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

	bufferDesc.ByteWidth = sizeof(CB_MODEL_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_constantBuffer));

	bufferDesc.ByteWidth = sizeof(CB_MODEL_ALPHA_THRESHOLD);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_alphaThresholdBuffer));
	
	V_RETURN(_dsStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	V_RETURN(_samplerStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	V_RETURN(_blendStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));
	V_RETURN(_rasterStates.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

	return S_OK;
}

void ModelRenderer::OnD3D11DestroyDevice()
{
	for (UINT i = 0; i < 2; i++)
	{
		for (UINT j = 0; j < 2; j++)
		{
			for (UINT k = 0; k < 2; k++)
			{
				SAFE_RELEASE(_meshPixelShader[i][j][k][0]);
				SAFE_RELEASE(_meshPixelShader[i][j][k][1]);
			}
		}		
	}

	SAFE_RELEASE(_meshVertexShader);
	SAFE_RELEASE(_meshInputLayout);
	
	SAFE_RELEASE(_constantBuffer);
	SAFE_RELEASE(_alphaThresholdBuffer);

	_dsStates.OnD3D11DestroyDevice();
	_samplerStates.OnD3D11DestroyDevice();
	_blendStates.OnD3D11DestroyDevice();
	_rasterStates.OnD3D11DestroyDevice();
}

HRESULT ModelRenderer::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(_dsStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_samplerStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_blendStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_rasterStates.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void ModelRenderer::OnD3D11ReleasingSwapChain()
{
	_dsStates.OnD3D11ReleasingSwapChain();
	_samplerStates.OnD3D11ReleasingSwapChain();
	_blendStates.OnD3D11ReleasingSwapChain();
	_rasterStates.OnD3D11ReleasingSwapChain();
}
