#include "PointLightRenderer.h"
#include "ShaderLoader.h"

const float PointLightRenderer::BIAS = 0.02f;

PointLightRenderer::PointLightRenderer()
	: _depthVS(NULL), _depthInput(NULL), _depthPropertiesBuffer(NULL),
	  _vertexShader(NULL), _unshadowedPS(NULL), _shadowedPS(NULL), _modelPropertiesBuffer(NULL),
	  _lightPropertiesBuffer(NULL), _cameraPropertiesBuffer(NULL), _lightInputLayout(NULL),
	  _lightModel(L"\\models\\sphere\\sphere.sdkmesh")
{
	for (UINT i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		_shadowMapTextures[i] = NULL;
		_shadowMapDSVs[i] = NULL;
		_shadowMapSRVs[i] = NULL;
	}
}

HRESULT PointLightRenderer::RenderShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, std::vector<ModelInstance*>* models,
	Camera* camera, AxisAlignedBox* sceneBounds)
{
	if (GetCount(true) > 0)
	{
		D3DPERF_BeginEvent(D3DCOLOR_COLORVALUE(0.0f, 1.0f, 0.0f, 1.0f), L"Point Light Shadow Maps");

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
		
		D3DPERF_EndEvent();
	}
	return S_OK;
}

HRESULT PointLightRenderer::renderDepth(ID3D11DeviceContext* pd3dImmediateContext, PointLight* light,
	UINT shadowMapIdx, std::vector<ModelInstance*>* models, Camera* camera, AxisAlignedBox* sceneBounds)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;	

	// Create a bounding sphere for the light
	Sphere lightSphere;
	lightSphere.Center = light->Position;
	lightSphere.Radius = light->Radius;

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

	// Set up the render targets for the shadow map and clear them
	pd3dImmediateContext->OMSetRenderTargets(0, NULL, _shadowMapDSVs[shadowMapIdx]);
	pd3dImmediateContext->ClearDepthStencilView(_shadowMapDSVs[shadowMapIdx], D3D11_CLEAR_DEPTH, 1.0f, 0);
		
	pd3dImmediateContext->GSSetShader(NULL, NULL, 0);
	pd3dImmediateContext->VSSetShader(_depthVS, NULL, 0);
	pd3dImmediateContext->PSSetShader(NULL, NULL, 0);	

	pd3dImmediateContext->IASetInputLayout(_depthInput);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

	// Create view matrix
	XMVECTOR lightPos = XMLoadFloat3(&light->Position);
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
		depthProperties->CameraClips = XMFLOAT2(0.1f, light->Radius);

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

			model->RenderMesh(pd3dImmediateContext, j);
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
		depthProperties->CameraClips = XMFLOAT2(0.1f, light->Radius);

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

			model->RenderMesh(pd3dImmediateContext, j);
		}
	}
	
	XMStoreFloat4x4(&_shadowMatricies[shadowMapIdx], XMMatrixTranspose(view));

	return S_OK;
}

HRESULT PointLightRenderer::RenderLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
	GBuffer* gBuffer)
{
	if (GetCount(true) + GetCount(false) > 0)
	{
		D3DPERF_BeginEvent(D3DCOLOR_COLORVALUE(0.0f, 1.0f, 0.0f, 1.0f), L"Point Lights");

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

		V_RETURN(gBuffer->PSSetShaderResources(pd3dImmediateContext, 0));	

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
			lightBounds.Center = light->Position;
			lightBounds.Radius = light->Radius;

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

			_lightModel.SetPosition(light->Position);
			_lightModel.SetScale(light->Radius);

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

			lightProperties->LightColor = light->Color;
			lightProperties->LightPosition = light->Position;
			lightProperties->LightRadius = light->Radius;

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
			lightBounds.Center = light->Position;
			lightBounds.Radius = light->Radius;
			
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

			_lightModel.SetPosition(light->Position);
			_lightModel.SetScale(light->Radius);

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

			lightProperties->LightColor = light->Color;
			lightProperties->LightPosition = light->Position;
			lightProperties->LightRadius = light->Radius;

			pd3dImmediateContext->Unmap(_lightPropertiesBuffer, 0);

			pd3dImmediateContext->PSSetConstantBuffers(2, 1, &_lightPropertiesBuffer);

			// Map the shadow properties
			V_RETURN(pd3dImmediateContext->Map(_shadowPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			CB_POINTLIGHT_SHADOW_PROPERTIES* shadowProperties = 
				(CB_POINTLIGHT_SHADOW_PROPERTIES*)mappedResource.pData;
		
			// Shadow matrix is already transposed
			shadowProperties->ShadowMatrix = _shadowMatricies[i];
			shadowProperties->CameraClips = XMFLOAT2(0.1f, light->Radius);
			shadowProperties->ShadowMapSize = XMFLOAT2((float)SHADOW_MAP_SIZE, (float)SHADOW_MAP_SIZE * 0.5f);
			shadowProperties->Bias = BIAS;			

			pd3dImmediateContext->Unmap(_shadowPropertiesBuffer, 0);

			pd3dImmediateContext->PSSetConstantBuffers(3, 1, &_shadowPropertiesBuffer);

			// Set the shadow map
			pd3dImmediateContext->PSSetShaderResources(5, 1, &_shadowMapSRVs[i]);

			Model* model = _lightModel.GetModel();
			model->Render(pd3dImmediateContext);
		}

		// Null all the SRVs
		ID3D11ShaderResourceView* nullSRV[5] = { NULL, NULL, NULL, NULL, NULL };
		pd3dImmediateContext->PSSetShaderResources(0, 5, nullSRV);

		// Reset the raster state
		pd3dImmediateContext->RSSetState(prevRS);
		SAFE_RELEASE(prevRS);

		D3DPERF_EndEvent();
	}
	return S_OK;
}

HRESULT PointLightRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	// Call base function
	V_RETURN(LightRenderer::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	V_RETURN(_lightModel.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

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

	bufferDesc.ByteWidth = sizeof(CB_POINTLIGHT_LIGHT_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_lightPropertiesBuffer));

	bufferDesc.ByteWidth = sizeof(CB_POINTLIGHT_CAMERA_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_cameraPropertiesBuffer));

	bufferDesc.ByteWidth = sizeof(CB_POINTLIGHT_SHADOW_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_shadowPropertiesBuffer));

	bufferDesc.ByteWidth = sizeof(CB_POINTLIGHT_DEPTH_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_depthPropertiesBuffer));

	// Create the shaders and input layout
	ID3DBlob* pBlob = NULL;

	V_RETURN(CompileShaderFromFile(L"PointLight.hlsl", "PS_PointLightUnshadowed", "ps_4_0", NULL, &pBlob));   
	V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_unshadowedPS));
	SAFE_RELEASE(pBlob);

	V_RETURN(CompileShaderFromFile(L"PointLight.hlsl", "PS_PointLightShadowed", "ps_4_0", NULL, &pBlob));   
	V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_shadowedPS));
	SAFE_RELEASE(pBlob);	

	V_RETURN(CompileShaderFromFile(L"PointLight.hlsl", "VS_PointLight", "vs_4_0", NULL, &pBlob));   
	V_RETURN(pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_vertexShader));

	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	V_RETURN( pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(), &_lightInputLayout));
	SAFE_RELEASE(pBlob);

	V_RETURN(CompileShaderFromFile(L"DualParaboloidDepth.hlsl", "VS_Depth", "vs_4_0", NULL, &pBlob));   
	V_RETURN(pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_depthVS));

	V_RETURN( pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(), &_depthInput));
	SAFE_RELEASE(pBlob);

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
		V_RETURN(pd3dDevice->CreateShaderResourceView(_shadowMapTextures[i], &shadowMapSRVDesc, &_shadowMapSRVs[i]));		
		V_RETURN(pd3dDevice->CreateDepthStencilView(_shadowMapTextures[i], &shadowMapDSVDesc, &_shadowMapDSVs[i]));
	}

	return S_OK;
}

void PointLightRenderer::OnD3D11DestroyDevice()
{
	LightRenderer::OnD3D11DestroyDevice();

	_lightModel.OnD3D11DestroyDevice();

	SAFE_RELEASE(_depthVS);
	SAFE_RELEASE(_depthInput);
	SAFE_RELEASE(_depthPropertiesBuffer);

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

HRESULT PointLightRenderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(LightRenderer::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	
	V_RETURN(_lightModel.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void PointLightRenderer::OnD3D11ReleasingSwapChain()
{
	LightRenderer::OnD3D11ReleasingSwapChain();

	_lightModel.OnD3D11ReleasingSwapChain();
}