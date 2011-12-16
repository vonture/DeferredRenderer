#include "PCH.h"
#include "DualParaboloidPointLightRenderer.h"
#include "PixelShaderLoader.h"
#include "VertexShaderLoader.h"
#include "Logger.h"

const float DualParaboloidPointLightRenderer::BIAS = 0.02f;

DualParaboloidPointLightRenderer::DualParaboloidPointLightRenderer()
	: _depthPS(NULL), _alphaCutoutPropertiesBuffer(NULL), _depthPropertiesBuffer(NULL),
	  _vertexShader(NULL), _unshadowedPS(NULL), _shadowedPS(NULL), _modelPropertiesBuffer(NULL),
	  _lightPropertiesBuffer(NULL), _cameraPropertiesBuffer(NULL), _lightInputLayout(NULL),
	  _lightModel(L"\\models\\sphere\\sphere.sdkmesh")
{
	for (UINT i = 0; i < 2; i++)
	{
		_depthVS[i] = NULL;
		_depthInput[i] = NULL;
	}

	for (UINT i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		_shadowMapTextures[i] = NULL;
		_shadowMapDSVs[i] = NULL;
		_shadowMapSRVs[i] = NULL;
	}
}

HRESULT DualParaboloidPointLightRenderer::RenderShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, 
	std::vector<ModelInstance*>* models, Camera* camera, AxisAlignedBox* sceneBounds)
{
	if (GetCount(true) > 0)
	{
		BEGIN_EVENT_D3D(L"Point Light Shadow Maps");

		// Save the old viewport
		D3D11_VIEWPORT vpOld[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
		UINT nViewPorts = 1;
		pd3dImmediateContext->RSGetViewports(&nViewPorts, vpOld);

		// Iterate over the lights and render the shadow maps
		for (UINT i = 0; i < GetCount(true) && i < NUM_SHADOW_MAPS; i++)
		{
			renderDepth(pd3dImmediateContext, GetLight(i, true), i, models, camera, sceneBounds);
		}

		// Re-apply the old viewport
		pd3dImmediateContext->RSSetViewports(nViewPorts, vpOld);
		
		END_EVENT_D3D(L"");
	}
	return S_OK;
}

HRESULT DualParaboloidPointLightRenderer::renderDepth(ID3D11DeviceContext* pd3dImmediateContext, PointLight* light,
	UINT shadowMapIdx, std::vector<ModelInstance*>* models, Camera* camera, AxisAlignedBox* sceneBounds)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;	

	// Create a bounding sphere for the light
	Sphere lightSphere;
	lightSphere.Center = light->GetPosition();
	lightSphere.Radius = light->GetRadius();

	// Make sure this light is in the view fustrum
	XMFLOAT4X4 fProj = camera->GetProjection();
	XMMATRIX proj = XMLoadFloat4x4(&fProj);

	Frustum cameraFrust;
	Collision::ComputeFrustumFromProjection(&cameraFrust, &proj);
	cameraFrust.Origin = camera->GetPosition();
	cameraFrust.Orientation = camera->GetOrientation();
	if (!Collision::IntersectSphereFrustum(&lightSphere, &cameraFrust))
	{
		return S_OK;
	}

	bool alphaCutoutEnabled = GetAlphaCutoutEnabled();

	// Set up the render targets for the shadow map and clear them
	pd3dImmediateContext->OMSetRenderTargets(0, NULL, _shadowMapDSVs[shadowMapIdx]);
	pd3dImmediateContext->ClearDepthStencilView(_shadowMapDSVs[shadowMapIdx], D3D11_CLEAR_DEPTH, 1.0f, 0);
		
	pd3dImmediateContext->GSSetShader(NULL, NULL, 0);
	pd3dImmediateContext->VSSetShader(alphaCutoutEnabled ? _depthVS[1] : _depthVS[0], NULL, 0);
	pd3dImmediateContext->PSSetShader(alphaCutoutEnabled ? _depthPS : NULL, NULL, 0);	

	pd3dImmediateContext->IASetInputLayout(alphaCutoutEnabled ? _depthInput[1] : _depthInput[0]);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

	// Alpha cutout settings
	if (alphaCutoutEnabled)
	{
		ID3D11SamplerState* samplers[1] = { GetSamplerStates()->GetAnisotropic16Wrap() };
		pd3dImmediateContext->PSSetSamplers(0, 1, samplers);
				
		V_RETURN(pd3dImmediateContext->Map(_alphaCutoutPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_POINTLIGHT_ALPHACUTOUT_PROPERTIES* modelProperties = (CB_POINTLIGHT_ALPHACUTOUT_PROPERTIES*)mappedResource.pData;
		modelProperties->AlphaThreshold = GetAlphaThreshold();
		pd3dImmediateContext->Unmap(_alphaCutoutPropertiesBuffer, 0);

		pd3dImmediateContext->PSSetConstantBuffers(1, 1, &_alphaCutoutPropertiesBuffer);			
	}

	// Create view matrix
	XMVECTOR lightPos = XMLoadFloat3(&light->GetPosition());
	XMVECTOR lightForward = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

	XMMATRIX view = XMMatrixLookToLH(lightPos, lightForward, lightUp);

	// Create the viewport
	D3D11_VIEWPORT vp;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = SHADOW_MAP_SIZE * 0.5f;
	vp.Height = SHADOW_MAP_SIZE * 0.5f;
	vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
	
	// Render the front depths
	pd3dImmediateContext->RSSetViewports(1, &vp);
		
	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthWriteEnabled(), 0);

	pd3dImmediateContext->RSSetState(GetRasterizerStates()->GetBackFaceCull());
	// Render the front depths
	for (UINT i = 0; i < models->size(); i++)
	{
		ModelInstance* instance = models->at(i);
		Model* model = instance->GetModel();

		// First a large check to see if any of the model is in the light's radius
		XMFLOAT4X4 fWorld = instance->GetWorld();
		XMMATRIX world = XMLoadFloat4x4(&fWorld);

		XMMATRIX wv = XMMatrixMultiply(world, view);

		V(pd3dImmediateContext->Map(_depthPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_POINTLIGHT_DEPTH_PROPERTIES* depthProperties = (CB_POINTLIGHT_DEPTH_PROPERTIES*)mappedResource.pData;

		XMStoreFloat4x4(&depthProperties->WorldView, XMMatrixTranspose(wv));
		depthProperties->Direction = 1.0f;
		depthProperties->CameraClips = XMFLOAT2(0.1f, light->GetRadius());

		pd3dImmediateContext->Unmap(_depthPropertiesBuffer, 0);

		pd3dImmediateContext->VSSetConstantBuffers(0, 1, &_depthPropertiesBuffer);

		for (UINT j = 0; j < model->GetMeshCount(); j++)
		{
			OrientedBox meshBounds = instance->GetMeshOrientedBox(j);
			
			// Make sure it's in the light radius
			if (!Collision::IntersectSphereOrientedBox(&lightSphere, &meshBounds))
			{
				continue;
			}

			model->RenderMesh(pd3dImmediateContext, j, INVALID_BUFFER_SLOT,
				alphaCutoutEnabled ? 0 : INVALID_SAMPLER_SLOT, INVALID_SAMPLER_SLOT, INVALID_SAMPLER_SLOT);
		}
	}
	
	// render the back depths
	vp.TopLeftX = SHADOW_MAP_SIZE * 0.5f;
	
	pd3dImmediateContext->RSSetViewports(1, &vp);
	
	pd3dImmediateContext->RSSetState(GetRasterizerStates()->GetFrontFaceCull());
	for (UINT i = 0; i < models->size(); i++)
	{
		ModelInstance* instance = models->at(i);
		Model* model = instance->GetModel();

		XMFLOAT4X4 fWorld = instance->GetWorld();
		XMMATRIX world = XMLoadFloat4x4(&fWorld);

		XMMATRIX wv = XMMatrixMultiply(world, view);

		V(pd3dImmediateContext->Map(_depthPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_POINTLIGHT_DEPTH_PROPERTIES* depthProperties = (CB_POINTLIGHT_DEPTH_PROPERTIES*)mappedResource.pData;

		XMStoreFloat4x4(&depthProperties->WorldView, XMMatrixTranspose(wv));
		depthProperties->Direction = -1.0f;
		depthProperties->CameraClips = XMFLOAT2(0.1f, light->GetRadius());

		pd3dImmediateContext->Unmap(_depthPropertiesBuffer, 0);

		pd3dImmediateContext->VSSetConstantBuffers(0, 1, &_depthPropertiesBuffer);

		for (UINT j = 0; j < model->GetMeshCount(); j++)
		{
			OrientedBox meshBounds = instance->GetMeshOrientedBox(j);
			
			// Make sure it's in the light radius
			if (!Collision::IntersectSphereOrientedBox(&lightSphere, &meshBounds))
			{
				continue;
			}

			model->RenderMesh(pd3dImmediateContext, j, INVALID_BUFFER_SLOT,
				alphaCutoutEnabled ? 0 : INVALID_SAMPLER_SLOT, INVALID_SAMPLER_SLOT, INVALID_SAMPLER_SLOT);
		}
	}
	
	XMStoreFloat4x4(&_shadowMatricies[shadowMapIdx], XMMatrixTranspose(view));

	return S_OK;
}

HRESULT DualParaboloidPointLightRenderer::RenderLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
	GBuffer* gBuffer)
{
	if (GetCount(true) + GetCount(false) > 0)
	{
		BEGIN_EVENT_D3D(L"Point Lights");

		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
	
		// prepare the camera properties buffer
		XMFLOAT4X4 fViewProj = camera->GetViewProjection();
		XMMATRIX cameraViewProj = XMLoadFloat4x4(&fViewProj);

		XMVECTOR det;
		XMMATRIX cameraInvViewProj = XMMatrixInverse(&det, cameraViewProj);

		XMFLOAT3 fCameraPos = camera->GetPosition();
		XMVECTOR cameraPos = XMLoadFloat3(&fCameraPos);

		// Set the global properties for all point lights
		ID3D11SamplerState* samplers[2] =
		{
			GetSamplerStates()->GetPointClamp(),
			GetSamplerStates()->GetShadowMap(),
		};
		pd3dImmediateContext->PSSetSamplers(0, 2, samplers);	

		float blendFactor[4] = {1, 1, 1, 1};
		pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetAdditiveBlend(), blendFactor, 0xFFFFFFFF);

		ID3D11ShaderResourceView* gBufferSRVs[3] = 
		{
			gBuffer->GetDiffuseSRV(),
			gBuffer->GetNormalSRV(),
			gBuffer->GetDepthSRV(),
		};
		pd3dImmediateContext->PSSetShaderResources(0, 3, gBufferSRVs);

		// Set the shaders and input
		pd3dImmediateContext->GSSetShader(NULL, NULL, 0);
		pd3dImmediateContext->VSSetShader(_vertexShader, NULL, 0);

		pd3dImmediateContext->IASetInputLayout(_lightInputLayout);

		// build the camera frustum
		XMFLOAT4X4 fProj = camera->GetProjection();
		XMMATRIX proj = XMLoadFloat4x4(&fProj);

		Frustum cameraFrust;
		Collision::ComputeFrustumFromProjection(&cameraFrust, &proj);
		cameraFrust.Origin = camera->GetPosition();
		cameraFrust.Orientation = camera->GetOrientation();

		// map the camera properties
		V_RETURN(pd3dImmediateContext->Map(_cameraPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_POINTLIGHT_CAMERA_PROPERTIES* cameraProperties = (CB_POINTLIGHT_CAMERA_PROPERTIES*)mappedResource.pData;
	
		XMStoreFloat4x4(&cameraProperties->InverseViewProjection,  XMMatrixTranspose(cameraInvViewProj));
		XMStoreFloat4(&cameraProperties->CameraPosition, cameraPos);

		pd3dImmediateContext->Unmap(_cameraPropertiesBuffer, 0);

		pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_cameraPropertiesBuffer);

		ID3D11RasterizerState* prevRS;
		pd3dImmediateContext->RSGetState(&prevRS);

		D3D11_RASTERIZER_DESC rsDesc;
		prevRS->GetDesc(&rsDesc);

		// Begin rendering unshadowed lights
		pd3dImmediateContext->PSSetShader(_unshadowedPS, NULL, 0);	

		int numUnshadowed = GetCount(false);
		for (int i = 0; i < numUnshadowed; i++)
		{
			PointLight* light = GetLight(i, false);

			// Verify that the light is visible
			Sphere lightBounds;
			lightBounds.Center = light->GetPosition();
			lightBounds.Radius = light->GetRadius();

			if (!Collision::IntersectSphereFrustum(&lightBounds, &cameraFrust))
			{
				continue;
			}
			
			// Depending on if the camera is within the light, flip the vertex winding
			if (Collision::IntersectPointSphere(cameraPos, &lightBounds))
			{
				pd3dImmediateContext->RSSetState(GetRasterizerStates()->GetFrontFaceCull());
				pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetReverseDepthEnabled(), 0);
			}
			else
			{
				pd3dImmediateContext->RSSetState(GetRasterizerStates()->GetBackFaceCull());
				pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthEnabled(), 0);
			}

			// Setup the model and map the model properties
			XMFLOAT4X4 fViewProj = camera->GetViewProjection();
			XMMATRIX viewProj = XMLoadFloat4x4(&fViewProj);

			_lightModel.SetPosition(light->GetPosition());
			_lightModel.SetScale(light->GetRadius());

			XMFLOAT4X4 fWorld = _lightModel.GetWorld();
			XMMATRIX world = XMLoadFloat4x4(&fWorld);

			XMMATRIX wvp = XMMatrixMultiply(world, viewProj);

			V_RETURN(pd3dImmediateContext->Map(_modelPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			CB_POINTLIGHT_MODEL_PROPERTIES* modelProperties = 
				(CB_POINTLIGHT_MODEL_PROPERTIES*)mappedResource.pData;

			XMStoreFloat4x4(&modelProperties->World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&modelProperties->WorldViewProjection, XMMatrixTranspose(wvp));

			pd3dImmediateContext->Unmap(_modelPropertiesBuffer, 0);

			pd3dImmediateContext->VSSetConstantBuffers(1, 1, &_modelPropertiesBuffer);

			// Map the light properties
			V_RETURN(pd3dImmediateContext->Map(_lightPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			CB_POINTLIGHT_LIGHT_PROPERTIES* lightProperties = 
				(CB_POINTLIGHT_LIGHT_PROPERTIES*)mappedResource.pData;
						
			lightProperties->LightPosition = light->GetPosition();
			lightProperties->LightRadius = light->GetRadius();
			lightProperties->LightColor = light->GetColor();
			lightProperties->LightBrightness = light->GetBrightness();

			pd3dImmediateContext->Unmap(_lightPropertiesBuffer, 0);
				
			pd3dImmediateContext->PSSetConstantBuffers(2, 1, &_lightPropertiesBuffer);
		
			Model* model = _lightModel.GetModel();
			model->Render(pd3dImmediateContext);
		}

		// Render the shadowed lights
		pd3dImmediateContext->PSSetShader(_shadowedPS, NULL, 0);	

		int numShadowed = GetCount(true);
		for (int i = 0; i < numShadowed; i++)
		{
			PointLight* light = GetLight(i, true);

			// Verify that the light is visible
			Sphere lightBounds;
			lightBounds.Center = light->GetPosition();
			lightBounds.Radius = light->GetRadius();
			
			if (!Collision::IntersectSphereFrustum(&lightBounds, &cameraFrust))
			{
				continue;
			}

			// Depending on if the camera is within the light, flip the vertex winding
			if (Collision::IntersectPointSphere(cameraPos, &lightBounds))
			{
				pd3dImmediateContext->RSSetState(GetRasterizerStates()->GetFrontFaceCull());
				pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetReverseDepthEnabled(), 0);
			}
			else
			{
				pd3dImmediateContext->RSSetState(GetRasterizerStates()->GetBackFaceCull());
				pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthEnabled(), 0);
			}

			// Setup the model and map the model properties
			XMFLOAT4X4 fViewProj = camera->GetViewProjection();
			XMMATRIX viewProj = XMLoadFloat4x4(&fViewProj);

			_lightModel.SetPosition(light->GetPosition());
			_lightModel.SetScale(light->GetRadius());

			XMFLOAT4X4 fWorld = _lightModel.GetWorld();
			XMMATRIX world = XMLoadFloat4x4(&fWorld);

			XMMATRIX wvp = XMMatrixMultiply(world, viewProj);

			V_RETURN(pd3dImmediateContext->Map(_modelPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			CB_POINTLIGHT_MODEL_PROPERTIES* modelProperties = 
				(CB_POINTLIGHT_MODEL_PROPERTIES*)mappedResource.pData;

			XMStoreFloat4x4(&modelProperties->World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&modelProperties->WorldViewProjection, XMMatrixTranspose(wvp));

			pd3dImmediateContext->Unmap(_modelPropertiesBuffer, 0);

			pd3dImmediateContext->VSSetConstantBuffers(1, 1, &_modelPropertiesBuffer);

			// Map the light properties
			V_RETURN(pd3dImmediateContext->Map(_lightPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			CB_POINTLIGHT_LIGHT_PROPERTIES* lightProperties = 
				(CB_POINTLIGHT_LIGHT_PROPERTIES*)mappedResource.pData;

			lightProperties->LightPosition = light->GetPosition();
			lightProperties->LightRadius = light->GetRadius();
			lightProperties->LightColor = light->GetColor();
			lightProperties->LightBrightness = light->GetBrightness();

			pd3dImmediateContext->Unmap(_lightPropertiesBuffer, 0);

			pd3dImmediateContext->PSSetConstantBuffers(2, 1, &_lightPropertiesBuffer);

			// Map the shadow properties
			V_RETURN(pd3dImmediateContext->Map(_shadowPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			CB_POINTLIGHT_SHADOW_PROPERTIES* shadowProperties = 
				(CB_POINTLIGHT_SHADOW_PROPERTIES*)mappedResource.pData;
		
			// Shadow matrix is already transposed
			shadowProperties->ShadowMatrix = _shadowMatricies[i];
			shadowProperties->CameraClips = XMFLOAT2(0.1f, light->GetRadius());
			shadowProperties->ShadowMapSize = XMFLOAT2((float)SHADOW_MAP_SIZE, (float)SHADOW_MAP_SIZE * 0.5f);
			shadowProperties->Bias = BIAS;			

			pd3dImmediateContext->Unmap(_shadowPropertiesBuffer, 0);

			pd3dImmediateContext->PSSetConstantBuffers(3, 1, &_shadowPropertiesBuffer);

			// Set the shadow map
			pd3dImmediateContext->PSSetShaderResources(3, 1, &_shadowMapSRVs[i]);

			Model* model = _lightModel.GetModel();
			model->Render(pd3dImmediateContext);
		}

		// Null all the SRVs
		ID3D11ShaderResourceView* nullSRV4[4] = { NULL, NULL, NULL, NULL };
		pd3dImmediateContext->PSSetShaderResources(0, 4, nullSRV4);

		// Reset the raster state
		pd3dImmediateContext->RSSetState(prevRS);
		SAFE_RELEASE(prevRS);

		END_EVENT_D3D(L"");
	}
	return S_OK;
}

HRESULT DualParaboloidPointLightRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	// Call base function
	V_RETURN(LightRenderer::OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

	V_RETURN(_lightModel.OnD3D11CreateDevice(pd3dDevice, pContentManager, pBackBufferSurfaceDesc));

	// Create the constant buffers
	D3D11_BUFFER_DESC bufferDesc =
	{
		0, //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	bufferDesc.ByteWidth = sizeof(CB_POINTLIGHT_MODEL_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_modelPropertiesBuffer));
	V_RETURN(SetDXDebugName(_modelPropertiesBuffer, "DP Light model CB"));

	bufferDesc.ByteWidth = sizeof(CB_POINTLIGHT_ALPHACUTOUT_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_alphaCutoutPropertiesBuffer));
	V_RETURN(SetDXDebugName(_modelPropertiesBuffer, "DP Light alpha cutout CB"));

	bufferDesc.ByteWidth = sizeof(CB_POINTLIGHT_LIGHT_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_lightPropertiesBuffer));
	V_RETURN(SetDXDebugName(_modelPropertiesBuffer, "DP Light light CB"));

	bufferDesc.ByteWidth = sizeof(CB_POINTLIGHT_CAMERA_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_cameraPropertiesBuffer));
	V_RETURN(SetDXDebugName(_modelPropertiesBuffer, "DP Light camera CB"));

	bufferDesc.ByteWidth = sizeof(CB_POINTLIGHT_SHADOW_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_shadowPropertiesBuffer));
	V_RETURN(SetDXDebugName(_modelPropertiesBuffer, "DP Light shadow CB"));

	bufferDesc.ByteWidth = sizeof(CB_POINTLIGHT_DEPTH_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_depthPropertiesBuffer));
	V_RETURN(SetDXDebugName(_modelPropertiesBuffer, "DP Light depth CB"));

	// Create the shaders and input layout
	PixelShaderContent* psContent = NULL;
	VertexShaderContent* vsContent = NULL;

	char entryPoint[256];
	char debugName[256];
	PixelShaderOptions psOpts =
	{
		entryPoint,// const char* EntryPoint;
		NULL,		// D3D_SHADER_MACRO* Defines;
		debugName,	// const char* DebugName;
	};

	D3D11_INPUT_ELEMENT_DESC posOnlyLayout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D11_INPUT_ELEMENT_DESC posTexcoordLayout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	VertexShaderOptions vsOpts = 
	{
		entryPoint,					// const char* EntryPoint;
		NULL,						// D3D_SHADER_MACRO* Defines;
		posOnlyLayout,				// D3D11_INPUT_ELEMENT_DESC* InputElements;
		ARRAYSIZE(posOnlyLayout),	// UINT InputElementCount;
		debugName,					// const char* DebugName;
	};

	// unshadowed ps
	sprintf_s(entryPoint, "PS_PointLightUnshadowed");
	sprintf_s(debugName, "Dual paraboloid unshadowed");

	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"PointLight.hlsl", &psOpts, &psContent));

	_unshadowedPS = psContent->PixelShader;
	_unshadowedPS->AddRef();

	SAFE_RELEASE(psContent);

	// shadowed ps
	sprintf_s(entryPoint, "PS_PointLightShadowed");
	sprintf_s(debugName, "Dual paraboloid shadowed");

	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"PointLight.hlsl", &psOpts, &psContent));

	_shadowedPS = psContent->PixelShader;
	_shadowedPS->AddRef();

	SAFE_RELEASE(psContent);
	
	// point light vs
	sprintf_s(entryPoint, "VS_PointLight");
	sprintf_s(debugName, "Dual paraboloid");
	vsOpts.InputElements = posOnlyLayout;
	vsOpts.InputElementCount = ARRAYSIZE(posOnlyLayout);

	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"PointLight.hlsl", &vsOpts, &vsContent));

	_vertexShader = vsContent->VertexShader;
	_vertexShader->AddRef();

	_lightInputLayout = vsContent->InputLayout;
	_lightInputLayout->AddRef();

	SAFE_RELEASE(vsContent);

	// depth shaders
	D3D_SHADER_MACRO macros[] = 
	{
		{ "ALPHA_CUTOUT", "" },
		NULL,
	};
	vsOpts.Defines = macros;
	psOpts.Defines = macros;

	// dual paraboloid depth (no alpha cutout)
	macros[0].Definition = "0";
	sprintf_s(entryPoint, "VS_Depth");
	sprintf_s(debugName, "Dual paraboloid depth (alpha cutout = 0)");
	vsOpts.InputElements = posOnlyLayout;
	vsOpts.InputElementCount = ARRAYSIZE(posOnlyLayout);

	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"DualParaboloidDepth.hlsl", &vsOpts, &vsContent));

	_depthVS[0] = vsContent->VertexShader;
	_depthVS[0]->AddRef();

	_depthInput[0] = vsContent->InputLayout;
	_depthInput[0]->AddRef();

	SAFE_RELEASE(vsContent);

	// dual paraboloid depth (with alpha cutout)
	macros[0].Definition = "1";
	sprintf_s(entryPoint, "VS_Depth");
	sprintf_s(debugName, "Dual paraboloid depth (alpha cutout = 1)");	
	vsOpts.InputElements = posTexcoordLayout;
	vsOpts.InputElementCount = ARRAYSIZE(posTexcoordLayout);

	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"DualParaboloidDepth.hlsl", &vsOpts, &vsContent));

	_depthVS[1] = vsContent->VertexShader;
	_depthVS[1]->AddRef();

	_depthInput[1] = vsContent->InputLayout;
	_depthInput[1]->AddRef();

	SAFE_RELEASE(vsContent);

	// dual paraboloid depth ps
	macros[0].Definition = "1";
	sprintf_s(entryPoint, "PS_Depth");
	sprintf_s(debugName, "Dual paraboloid depth");

	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"DualParaboloidDepth.hlsl", &psOpts, &psContent));

	_depthPS = psContent->PixelShader;
	_depthPS->AddRef();

	SAFE_RELEASE(psContent);	
	
	// Create the shadow textures
	D3D11_TEXTURE2D_DESC shadowMapTextureDesc = 
	{
		SHADOW_MAP_SIZE,//UINT Width;
		SHADOW_MAP_SIZE / 2,//UINT Height;
		1,//UINT MipLevels;
		1,//UINT ArraySize;
		DXGI_FORMAT_R32_TYPELESS,//DXGI_FORMAT Format;
		1,//DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	};

	// Create the shadow map SRVs
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowMapSRVDesc = 
	{
		DXGI_FORMAT_R32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};
	shadowMapSRVDesc.Texture2D.MipLevels = 1;

	// Create the shadow map depth stencil views
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowMapDSVDesc =
	{
		DXGI_FORMAT_D32_FLOAT,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		0,
	};

	for (UINT i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		V_RETURN(pd3dDevice->CreateTexture2D(&shadowMapTextureDesc, NULL, &_shadowMapTextures[i]));
		sprintf_s(debugName, "DP shadow map %i texture", i);
		V_RETURN(SetDXDebugName(_shadowMapTextures[i], debugName));

		V_RETURN(pd3dDevice->CreateShaderResourceView(_shadowMapTextures[i], &shadowMapSRVDesc, &_shadowMapSRVs[i]));		
		sprintf_s(debugName, "DP shadow map %i SRV", i);
		V_RETURN(SetDXDebugName(_shadowMapSRVs[i], debugName));

		V_RETURN(pd3dDevice->CreateDepthStencilView(_shadowMapTextures[i], &shadowMapDSVDesc, &_shadowMapDSVs[i]));
		sprintf_s(debugName, "DP shadow map %i DSV", i);
		V_RETURN(SetDXDebugName(_shadowMapDSVs[i], debugName));
	}

	return S_OK;
}

void DualParaboloidPointLightRenderer::OnD3D11DestroyDevice()
{
	LightRenderer::OnD3D11DestroyDevice();

	_lightModel.OnD3D11DestroyDevice();

	for (UINT i = 0; i < 2; i++)
	{
		SAFE_RELEASE(_depthVS[i])
		SAFE_RELEASE(_depthInput[i]);
	}
	SAFE_RELEASE(_depthPS);
	SAFE_RELEASE(_depthPropertiesBuffer);
	SAFE_RELEASE(_alphaCutoutPropertiesBuffer);
	
	SAFE_RELEASE(_vertexShader);
	SAFE_RELEASE(_unshadowedPS);
	SAFE_RELEASE(_shadowedPS);

	SAFE_RELEASE(_modelPropertiesBuffer);
	SAFE_RELEASE(_lightPropertiesBuffer);
	SAFE_RELEASE(_cameraPropertiesBuffer);
	SAFE_RELEASE(_shadowPropertiesBuffer);
	SAFE_RELEASE(_lightInputLayout);

	for (UINT i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		SAFE_RELEASE(_shadowMapTextures[i]);
		SAFE_RELEASE(_shadowMapDSVs[i]);
		SAFE_RELEASE(_shadowMapSRVs[i]);
	}
}

HRESULT DualParaboloidPointLightRenderer::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(LightRenderer::OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));
	
	V_RETURN(_lightModel.OnD3D11ResizedSwapChain(pd3dDevice, pContentManager, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void DualParaboloidPointLightRenderer::OnD3D11ReleasingSwapChain()
{
	LightRenderer::OnD3D11ReleasingSwapChain();

	_lightModel.OnD3D11ReleasingSwapChain();
}