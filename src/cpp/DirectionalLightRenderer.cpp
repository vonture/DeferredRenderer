#include "DirectionalLightRenderer.h"

const float DirectionalLightRenderer::CASCADE_SPLITS[NUM_CASCADES] = { 0.125f, 0.25f, 0.5f, 1.0f };
const float DirectionalLightRenderer::BACKUP = 20.0f;
const float DirectionalLightRenderer::BIAS = 0.002f;

DirectionalLightRenderer::DirectionalLightRenderer()
	: _depthVS(NULL), _depthInput(NULL), _depthPropertiesBuffer(NULL),  _unshadowedPS(NULL),
	  _shadowedPS(NULL), _cameraPropertiesBuffer(NULL), _lightPropertiesBuffer(NULL),
	  _shadowPropertiesBuffer(NULL)
{
	for (int i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		_shadowMapTextures[i] = NULL;
		_shadowMapDSVs[i] = NULL;
		_shadowMapSRVs[i] = NULL;
	}
}

HRESULT DirectionalLightRenderer::RenderShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, 
	std::vector<ModelInstance*>* models, Camera* camera, BoundingBox* sceneBounds)
{
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
	
	return S_OK;
}

HRESULT DirectionalLightRenderer::renderDepth(ID3D11DeviceContext* pd3dImmediateContext, DirectionalLight* dlight,
	UINT shadowMapIdx, std::vector<ModelInstance*>* models, Camera* camera, BoundingBox* sceneBounds)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Set up the render targets for the shadow map and clear them
	pd3dImmediateContext->OMSetRenderTargets(0, NULL, _shadowMapDSVs[shadowMapIdx]);
	pd3dImmediateContext->ClearDepthStencilView(_shadowMapDSVs[shadowMapIdx], D3D11_CLEAR_DEPTH, 1.0f, 0);
		
	pd3dImmediateContext->GSSetShader(NULL, NULL, 0);
	pd3dImmediateContext->VSSetShader(_depthVS, NULL, 0);
	pd3dImmediateContext->PSSetShader(NULL, NULL, 0);

	pd3dImmediateContext->IASetInputLayout(_depthInput);
	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthWriteEnabled(), 0);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetBlendDisabled(), blendFactor, 0xFFFFFFFF);

	pd3dImmediateContext->RSSetState(GetRasterizerStates()->GetBackFaceCull());

	// Cascade offsets
    const XMFLOAT2 offsets[4] = {
        XMFLOAT2(0.0f, 0.0f),
        XMFLOAT2(0.5f, 0.0f),
        XMFLOAT2(0.5f, 0.5f),
        XMFLOAT2(0.0f, 0.5f)
    };

	for (UINT i = 0; i < NUM_CASCADES; i++)
	{
		// Create the viewport
		int numRows = (int)sqrtf((float)NUM_CASCADES);
        float cascadeSize =  (float)SHADOW_MAP_SIZE / numRows;

		D3D11_VIEWPORT vp;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.Width = cascadeSize;
		vp.Height = cascadeSize;
		vp.TopLeftX = offsets[i].x * cascadeSize * 2.0f;
        vp.TopLeftY = offsets[i].y * cascadeSize * 2.0f;

		pd3dImmediateContext->RSSetViewports(1, &vp);

		// calc the split depths
		float prevSplitDist = i == 0 ? 0.0f : CASCADE_SPLITS[i - 1];
        float splitDist = CASCADE_SPLITS[i];

		// Create the frustum
		BoundingFrustum lightFrust = BoundingFrustum(*camera->GetViewProjection());
	
		XMVECTOR frustCorners[8];
		lightFrust.GetCorners(frustCorners);

		// Scale by the shadow view distance
        for(UINT j = 0; j < 4; j++)
        {
            XMVECTOR cornerRay = XMVectorSubtract(frustCorners[j + 4], frustCorners[j]);
            XMVECTOR nearCornerRay = XMVectorScale(cornerRay, prevSplitDist);
            XMVECTOR farCornerRay = XMVectorScale(cornerRay, splitDist);
            frustCorners[j + 4] = XMVectorAdd(frustCorners[j], farCornerRay);
            frustCorners[j] = XMVectorAdd(frustCorners[j], nearCornerRay);
        }

		// Calculate the centroid of the view frustum
		BoundingBox viewFrustBox;
		BoundingBox::CreateFromPoints(&viewFrustBox, frustCorners, 8);

		XMVECTOR bbRad = XMVectorRound(XMVectorSubtract(*viewFrustBox.GetMax(), *viewFrustBox.GetMin()) * 0.5f);
		XMVECTOR bbMid = XMVectorRound(XMVectorAdd(*viewFrustBox.GetMax(), *viewFrustBox.GetMin()) * 0.5f);
		
		const float bbRadius = XMVectorGetX(XMVector3Length(bbRad));
        const float backupDist = bbRadius + camera->GetNearClip() + BACKUP;
		
		// Get position of the shadow camera
        XMVECTOR shadowCameraPosVec = bbMid;
        XMVECTOR backupDirVec = *dlight->GetDirection();
        backupDirVec = XMVectorScale(backupDirVec, backupDist);
        shadowCameraPosVec = XMVectorRound(XMVectorAdd(shadowCameraPosVec, backupDirVec));

		XMFLOAT3 bbCenter, shadowCameraPos;
        XMStoreFloat3(&bbCenter, bbMid);
        XMStoreFloat3(&shadowCameraPos, shadowCameraPosVec);
        XMFLOAT3 up(0.0f, 1.0f, 0.0f);

		// Set the camera perameters
		OrthographicCamera shadowCamera;
		shadowCamera.SetNearClip(camera->GetNearClip());
		shadowCamera.SetFarClip(backupDist + bbRadius);
		shadowCamera.SetMinX(-bbRadius);
		shadowCamera.SetMinY(-bbRadius);
		shadowCamera.SetMaxX(bbRadius);
		shadowCamera.SetMaxY(bbRadius);
		shadowCamera.SetLookAt(shadowCameraPos, bbCenter, up);

		// Create the rounding matrix, by projecting the world-space origin and determining
        // the fractional offset in texel space
        XMMATRIX shadowMatrix = *shadowCamera.GetViewProjection();
        XMVECTOR shadowOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        shadowOrigin = XMVector4Transform(shadowOrigin, shadowMatrix);
        shadowOrigin = XMVectorScale(shadowOrigin, cascadeSize / 2.0f);

        XMVECTOR roundedOrigin = XMVectorRound(shadowOrigin);
        XMVECTOR roundOffset = XMVectorSubtract(roundedOrigin, shadowOrigin);
        roundOffset = XMVectorScale(roundOffset, 2.0f / cascadeSize);
        roundOffset = XMVectorSetZ(roundOffset, 0.0f);
        roundOffset = XMVectorSetW(roundOffset, 0.0f);
		
        XMMATRIX shadowProj = *shadowCamera.GetProjection();
        shadowProj.r[3] = XMVectorAdd(shadowProj.r[3], roundOffset);
        shadowCamera.SetProjection(shadowProj);
        shadowMatrix = *shadowCamera.GetViewProjection();

		// Render the depth of all the models in the scene
		for (UINT j = 0; j < models->size(); j++)
		{
			ModelInstance* instance = models->at(j);

			XMMATRIX wvp = XMMatrixMultiply(*instance->GetWorld(), *shadowCamera.GetViewProjection());
		
			V_RETURN(pd3dImmediateContext->Map(_depthPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			CB_DIRECTIONALLIGHT_DEPTH_PROPERTIES* modelProperties = (CB_DIRECTIONALLIGHT_DEPTH_PROPERTIES*)mappedResource.pData;
			modelProperties->WorldViewProjection = XMMatrixTranspose(wvp);
			pd3dImmediateContext->Unmap(_depthPropertiesBuffer, 0);

			pd3dImmediateContext->VSSetConstantBuffers(0, 1, &_depthPropertiesBuffer);

			CDXUTSDKMesh* mesh = instance->GetMesh();
			mesh->Render(pd3dImmediateContext);
		}
		
		// Apply the scale/offset/bias matrix, which transforms from [-1,1]
        // post-projection space to [0,1] UV space
        XMMATRIX texScaleBias;
        texScaleBias.r[0] = XMVectorSet(0.5f,  0.0f, 0.0f, 0.0f);
        texScaleBias.r[1] = XMVectorSet(0.0f, -0.5f, 0.0f, 0.0f);
        texScaleBias.r[2] = XMVectorSet(0.0f,  0.0f, 1.0f, 0.0f);
        texScaleBias.r[3] = XMVectorSet(0.5f,  0.5f, -BIAS, 1.0f);
        shadowMatrix = XMMatrixMultiply(shadowMatrix, texScaleBias);

		// Apply the cascade offset/scale matrix, which applies the offset and scale needed to
        // convert the UV coordinate into the proper coordinate for the cascade being sampled in
        // the atlas.
        XMFLOAT4 offset = XMFLOAT4(offsets[i].x, offsets[i].y, 0.0f, 1.0);
        XMMATRIX cascadeOffsetMatrix = XMMatrixScaling(0.5f, 0.5f, 1.0f);
        cascadeOffsetMatrix.r[3] = XMLoadFloat4(&offset);
        shadowMatrix = XMMatrixMultiply(shadowMatrix, cascadeOffsetMatrix);
		
		// Store the shadow matrix and split depth
        _shadowMatricies[shadowMapIdx][i] = shadowMatrix;

		const float clipDist = camera->GetFarClip() - camera->GetNearClip();
		_cascadeSplits[shadowMapIdx][i] = camera->GetNearClip() + (splitDist * clipDist);
	}

	return S_OK;
}

HRESULT DirectionalLightRenderer::RenderLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
	GBuffer* gBuffer)
{	
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	
	// prepare the camera properties buffer
	XMVECTOR det;
	XMMATRIX cameraInvViewProj = XMMatrixInverse(&det, *camera->GetViewProjection());
	XMVECTOR cameraPos = camera->GetPosition();

	// Set the global properties for all directional lights
	ID3D11SamplerState* samplers[2] =
	{
		GetSamplerStates()->GetPoint(),
		GetSamplerStates()->GetShadowMap(),
	};
	pd3dImmediateContext->PSSetSamplers(0, 2, samplers);
	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetReverseDepthEnabled(), 0);	

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetAdditiveBlend(), blendFactor, 0xFFFFFFFF);

	V_RETURN(gBuffer->PSSetShaderResources(pd3dImmediateContext, 0));

	// map the camera properties
	V_RETURN(pd3dImmediateContext->Map(_cameraPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_DIRECTIONALLIGHT_CAMERA_PROPERTIES* cameraProperties = (CB_DIRECTIONALLIGHT_CAMERA_PROPERTIES*)mappedResource.pData;
	
	cameraProperties->InverseViewProjection = XMMatrixTranspose(cameraInvViewProj);
	XMStoreFloat4(&cameraProperties->CameraPosition, cameraPos);

	pd3dImmediateContext->Unmap(_cameraPropertiesBuffer, 0);

	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_cameraPropertiesBuffer);

	// Begin rendering unshadowed lights
	int numUnshadowed = GetCount(false);
	for (int i = 0; i < numUnshadowed; i++)
	{
		DirectionalLight* light = GetLight(i, false);

		V_RETURN(pd3dImmediateContext->Map(_lightPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_DIRECTIONALLIGHT_LIGHT_PROPERTIES* lightProperties = 
			(CB_DIRECTIONALLIGHT_LIGHT_PROPERTIES*)mappedResource.pData;

		XMStoreFloat4(&lightProperties->LightColor, *light->GetColor());
		XMStoreFloat3(&lightProperties->LightDirection, *light->GetDirection());
		lightProperties->LightIntensity = light->GetItensity();

		pd3dImmediateContext->Unmap(_lightPropertiesBuffer, 0);

		pd3dImmediateContext->PSSetConstantBuffers(1, 1, &_lightPropertiesBuffer);

		_fsQuad.Render(pd3dImmediateContext, _unshadowedPS);
	}
	
	// begin rendering shadowed lights
	int numShadowed = GetCount(true);
	for (int i = 0; i < numShadowed; i++)
	{
		DirectionalLight* light = GetLight(i, true);

		// Prepare the light properties
		V_RETURN(pd3dImmediateContext->Map(_lightPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_DIRECTIONALLIGHT_LIGHT_PROPERTIES* lightProperties = 
			(CB_DIRECTIONALLIGHT_LIGHT_PROPERTIES*)mappedResource.pData;

		XMStoreFloat4(&lightProperties->LightColor, *light->GetColor());
		XMStoreFloat3(&lightProperties->LightDirection, *light->GetDirection());
		lightProperties->LightIntensity = light->GetItensity();

		pd3dImmediateContext->Unmap(_lightPropertiesBuffer, 0);
		
		// Prepare the shadow properties
		V_RETURN(pd3dImmediateContext->Map(_shadowPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_DIRECTIONALLIGHT_SHADOW_PROPERTIES* shadowProperties = 
			(CB_DIRECTIONALLIGHT_SHADOW_PROPERTIES*)mappedResource.pData;

		for (UINT j = 0; j < NUM_CASCADES; j++)
		{
			shadowProperties->CascadeSplits[j] = _cascadeSplits[i][j];
			shadowProperties->ShadowMatricies[j] = XMMatrixTranspose(_shadowMatricies[i][j]);
		}
		shadowProperties->CameraClips = XMFLOAT2(camera->GetNearClip(), camera->GetFarClip());
		shadowProperties->ShadowMapSize = XMFLOAT2((float)SHADOW_MAP_SIZE, (float)SHADOW_MAP_SIZE);

		pd3dImmediateContext->Unmap(_shadowPropertiesBuffer, 0);

		// Set both constant buffers back to the shader at once
		ID3D11Buffer* constantBuffers[2] = 
		{
			_lightPropertiesBuffer,
			_shadowPropertiesBuffer
		};
		pd3dImmediateContext->PSSetConstantBuffers(1, 2, constantBuffers);

		// Set the shadow map SRV
		pd3dImmediateContext->PSSetShaderResources(5, 1, &_shadowMapSRVs[i]);

		// Finally, render the quad
		_fsQuad.Render(pd3dImmediateContext, _shadowedPS);
	}

	// Unset the shadow map SRV
	ID3D11ShaderResourceView* nullSRV[1] = 
	{
		NULL,
	};
	pd3dImmediateContext->PSSetShaderResources(5, 1, nullSRV);

	return S_OK;
}

HRESULT DirectionalLightRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	// Call base function
	V_RETURN(LightRenderer::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	// Compile both shaders from file
	ID3DBlob* pBlob = NULL;
	
	V_RETURN(CompileShaderFromFile( L"DirectionalLight.hlsl", "PS_DirectionalLightUnshadowed", "ps_4_0", NULL, &pBlob ) );   
	V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_unshadowedPS));
	SAFE_RELEASE(pBlob);

	V_RETURN(CompileShaderFromFile( L"DirectionalLight.hlsl", "PS_DirectionalLightShadowed", "ps_4_0", NULL, &pBlob ) );   
	V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_shadowedPS));
	SAFE_RELEASE(pBlob);
	
	V_RETURN(CompileShaderFromFile(L"Depth.hlsl", "VS_Depth", "vs_4_0", NULL, &pBlob));   
	V_RETURN(pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_depthVS));

	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	V_RETURN( pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(), &_depthInput));
	SAFE_RELEASE(pBlob);

	// Create the buffers
	D3D11_BUFFER_DESC bufferDesc =
	{
		0, //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	bufferDesc.ByteWidth = sizeof(CB_DIRECTIONALLIGHT_DEPTH_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_depthPropertiesBuffer));

	bufferDesc.ByteWidth = sizeof(CB_DIRECTIONALLIGHT_CAMERA_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_cameraPropertiesBuffer));
	
	bufferDesc.ByteWidth = sizeof(CB_DIRECTIONALLIGHT_LIGHT_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_lightPropertiesBuffer));

	bufferDesc.ByteWidth = sizeof(CB_DIRECTIONALLIGHT_SHADOW_PROPERTIES);
	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_shadowPropertiesBuffer));

	// Create the shadow textures
	D3D11_TEXTURE2D_DESC shadowMapTextureDesc = 
	{
		SHADOW_MAP_SIZE,//UINT Width;
		SHADOW_MAP_SIZE,//UINT Height;
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
	
	// create the shadow map depth stencil view
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

	// Load the other IHasContents
	V_RETURN(_fsQuad.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_modelRenderer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));	

	return S_OK;
}

void DirectionalLightRenderer::OnD3D11DestroyDevice()
{
	LightRenderer::OnD3D11DestroyDevice();

	SAFE_RELEASE(_depthVS);
	SAFE_RELEASE(_depthInput);
	SAFE_RELEASE(_depthPropertiesBuffer);

	SAFE_RELEASE(_unshadowedPS);
	SAFE_RELEASE(_shadowedPS);
	SAFE_RELEASE(_lightPropertiesBuffer);
	SAFE_RELEASE(_cameraPropertiesBuffer);
	SAFE_RELEASE(_shadowPropertiesBuffer);

	for (UINT i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		SAFE_RELEASE(_shadowMapTextures[i]);
		SAFE_RELEASE(_shadowMapSRVs[i]);
		SAFE_RELEASE(_shadowMapDSVs[i]);
	}

	_fsQuad.OnD3D11DestroyDevice();
	_modelRenderer.OnD3D11DestroyDevice();
}

HRESULT DirectionalLightRenderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(LightRenderer::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	V_RETURN(_fsQuad.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_modelRenderer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void DirectionalLightRenderer::OnD3D11ReleasingSwapChain()
{
	LightRenderer::OnD3D11ReleasingSwapChain();
	_fsQuad.OnD3D11ReleasingSwapChain();
	_modelRenderer.OnD3D11ReleasingSwapChain();
}
