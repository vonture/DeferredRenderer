#include "DXUT.h"
#include "DirectionalLightRenderer.h"

DirectionalLightRenderer::DirectionalLightRenderer()
	: _unshadowedPS(NULL), _shadowedPS(NULL), _cameraPropertiesBuffer(NULL),
	  _lightPropertiesBuffer(NULL)
{
}

HRESULT DirectionalLightRenderer::RenderShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, std::vector<ModelInstance*> models,
		Camera* camera, BoundingBox* sceneBounds)
{
	return S_OK;
}

HRESULT DirectionalLightRenderer::renderDepth(ID3D11DeviceContext* pd3dImmediateContext, DirectionalLight* dlight,
	UINT shadowMapIdx, std::vector<ModelInstance*> models, Camera* camera, BoundingBox* sceneBounds,
	OrthographicCamera** outLightCameras)
{
	float splitDepths[NUM_CASCADES + 1];
	calcSplitDepths(splitDepths, camera->GetNearClip(), camera->GetFarClip());

	for (UINT i = 0; i < NUM_CASCADES; i++)
	{
		calcLightCamera(dlight, camera, splitDepths[i], splitDepths[i + 1], outLightCameras[i]);

		// Set up the render targets for the shadow map and clear them
		pd3dImmediateContext->OMSetRenderTargets(1, &_shadowMapRTVs[shadowMapIdx], _shadowMapDSView);
				
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		pd3dImmediateContext->ClearRenderTargetView(_shadowMapRTVs[shadowMapIdx], clearColor);
		pd3dImmediateContext->ClearDepthStencilView(_shadowMapDSView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		// Render the depth of all the models in the scene
		_modelRenderer.RenderDepth(pd3dImmediateContext, models, outLightCameras[i]);
	}

	return S_OK;
}

void DirectionalLightRenderer::calcSplitDepths(float* outSplits, float nearClip, float farClip)
{
	outSplits[0] = nearClip;
	outSplits[NUM_CASCADES] = farClip;
	const float splitConstant = 0.95f;
	float fNumCascades = (float)NUM_CASCADES;

	for (UINT i = 1; i < NUM_CASCADES; i++)
	{
		outSplits[i] = splitConstant * nearClip * powf(farClip / nearClip, i / fNumCascades) +
			(1.0f - splitConstant) * ((nearClip + (i / fNumCascades)) * (farClip - nearClip));
	}
}

void DirectionalLightRenderer::calcLightCamera(DirectionalLight* dlight, Camera* mainCamera, float minZ,
	float maxZ, OrthographicCamera* outCamera)
{
	// create a matrix with that will rotate in points the direction of the light
	D3DXVECTOR3 zero = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 lightDir = *dlight->GetDirection();
	D3DXVECTOR3 invLightDir = -lightDir;

	D3DXMATRIX lightRot;
	D3DXMatrixLookAtLH(&lightRot, &zero, &invLightDir, &up);

	// Get the corners of the frustum
	D3DXMATRIX clippedProj;
	mainCamera->BuildProjection(&clippedProj, minZ, maxZ);

	D3DXMATRIX clippedViewProj;
	D3DXMatrixMultiply(&clippedViewProj, mainCamera->GetView(), &clippedProj);

	BoundingFrustum clippedFrust = BoundingFrustum(clippedViewProj);

	D3DXVECTOR3 frustCorners[8];
	clippedFrust.GetCorners(frustCorners);

	// Transform the positions of the corners into the direction of the light
	D3DXVec3TransformCoordArray(frustCorners, sizeof(D3DXVECTOR3), frustCorners, sizeof(D3DXVECTOR3), 
		&lightRot, 8);

	// Find the smallest box around the points
	BoundingBox lightBox;
	BoundingBox::CreateFromPoints(&lightBox, frustCorners, 8);

	// Find the position of the light in light space
	D3DXVECTOR3 boxSize = *lightBox.GetMin() - *lightBox.GetMax();

	D3DXVECTOR3 lightPos = D3DXVECTOR3(boxSize.x * 0.5f, boxSize.y * 0.5f, lightBox.GetMin()->z);
	
	// Find the light position in world space
	D3DXMATRIX invLightRot;
	D3DXMatrixInverse(&invLightRot, NULL, &lightRot);

	D3DXVECTOR3 lightPosWS;
	D3DXVec3TransformCoord(&lightPosWS, &lightPos, &invLightRot);

	// Fill in the camera parameters
	D3DXQUATERNION lightDirQuat = D3DXQUATERNION(lightDir.x, lightDir.y, lightDir.z, 1.0f);

	outCamera->SetPosition(lightPos);
	outCamera->SetOrientation(lightDirQuat);
	outCamera->SetSize(D3DXVECTOR2(boxSize.x, boxSize.y));

	// TODO: Calculate this properly based off of the scene bounds
	outCamera->SetNearClip(-boxSize.z * 1.5f);
	outCamera->SetFarClip(boxSize.z * 1.5f);
}

HRESULT DirectionalLightRenderer::RenderLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
	GBuffer* gBuffer)
{	
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	
	// prepare the camera properties buffer
	D3DXMATRIX cameraInvViewProj = (*camera->GetView()) * (*camera->GetProjection());
	D3DXMatrixInverse(&cameraInvViewProj, NULL, &cameraInvViewProj);
	D3DXMatrixTranspose(&cameraInvViewProj, &cameraInvViewProj);

	D3DXVECTOR3 cameraPos = camera->GetPosition();

	pd3dImmediateContext->PSSetShader(_unshadowedPS, NULL, 0);

	ID3D11SamplerState* sampler = GetSamplerStates()->GetLinear();
	pd3dImmediateContext->PSSetSamplers(0, 1, &sampler);
	pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthDisabled(), 0);

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetAdditiveBlend(), blendFactor, 0xFFFFFFFF);

	V_RETURN(gBuffer->PSSetShaderResources(pd3dImmediateContext, 0));

	V_RETURN(pd3dImmediateContext->Map(_cameraPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_DIRECTIONALLIGHT_CAMERA_PROPERTIES* cameraProperties = (CB_DIRECTIONALLIGHT_CAMERA_PROPERTIES*)mappedResource.pData;
	cameraProperties->InverseViewProjection = cameraInvViewProj;
	cameraProperties->CameraPosition = D3DXVECTOR4(cameraPos, 1.0f);
	pd3dImmediateContext->Unmap(_cameraPropertiesBuffer, 0);

	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_cameraPropertiesBuffer);

	int numUnshadowed = GetCount(false);
	for (int i = 0; i < numUnshadowed; i++)
	{
		DirectionalLight* light = GetLight(i, false);

		V_RETURN(pd3dImmediateContext->Map(_lightPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_DIRECTIONALLIGHT_PROPERTIES* lightProperties = (CB_DIRECTIONALLIGHT_PROPERTIES*)mappedResource.pData;
		lightProperties->LightColor = *light->GetColor();
		lightProperties->LightDirection = *light->GetDirection();
		lightProperties->LightIntensity = light->GetItensity();
		pd3dImmediateContext->Unmap(_lightPropertiesBuffer, 0);

		pd3dImmediateContext->PSSetConstantBuffers(1, 1, &_lightPropertiesBuffer);

		_fsQuad.Render(pd3dImmediateContext, _unshadowedPS);
	}
	
	return S_OK;
}

HRESULT DirectionalLightRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	// Call base function
	V_RETURN(LightRenderer::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	// Compile both shaders from file
	ID3DBlob* pBlob = NULL;
	
	V_RETURN(CompileShaderFromFile( L"DirectionalLight.hlsl", "PS_DirectionalLightUnshadowed", "ps_4_0", &pBlob ) );   
	V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_unshadowedPS));
	SAFE_RELEASE(pBlob);

	V_RETURN(CompileShaderFromFile( L"DirectionalLight.hlsl", "PS_DirectionalLightShadowed", "ps_4_0", &pBlob ) );   
	V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_shadowedPS));
	SAFE_RELEASE(pBlob);
	
	// Create the buffers
	D3D11_BUFFER_DESC cameraPropertiesBufferDesc =
	{
		sizeof(CB_DIRECTIONALLIGHT_CAMERA_PROPERTIES), //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	V_RETURN(pd3dDevice->CreateBuffer(&cameraPropertiesBufferDesc, NULL, &_cameraPropertiesBuffer));

	D3D11_BUFFER_DESC lightPropertiesBufferDesc =
	{
		sizeof(CB_DIRECTIONALLIGHT_PROPERTIES), //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	V_RETURN(pd3dDevice->CreateBuffer(&lightPropertiesBufferDesc, NULL, &_lightPropertiesBuffer));

	// Create the shadow textures
	D3D11_TEXTURE2D_DESC shadowMapTextureDesc = 
	{
		SHADOW_MAP_SIZE,//UINT Width;
		SHADOW_MAP_SIZE,//UINT Height;
		1,//UINT MipLevels;
		1,//UINT ArraySize;
		DXGI_FORMAT_R32G32_FLOAT,//DXGI_FORMAT Format;
		1,//DXGI_SAMPLE_DESC SampleDesc;
		0,
		D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
		D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
		0,//UINT CPUAccessFlags;
		0//UINT MiscFlags;    
	};

	for (UINT i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		V_RETURN(pd3dDevice->CreateTexture2D(&shadowMapTextureDesc, NULL, &_shadowMapTextures[i]));
	}

	// Create the shadow map SRVs
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowMapSRVDesc = 
	{
		DXGI_FORMAT_R32G32_FLOAT,
		D3D11_SRV_DIMENSION_TEXTURE2D,
		0,
		0
	};
	shadowMapSRVDesc.Texture2D.MipLevels = 1;

	for (UINT i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		V_RETURN(pd3dDevice->CreateShaderResourceView(_shadowMapTextures[i], &shadowMapSRVDesc, &_shadowMapSRVs[i]));
	}

	// Create the shadow map RTVs
	D3D11_RENDER_TARGET_VIEW_DESC shadowMapRTVDesc = 
	{
		DXGI_FORMAT_R32G32_FLOAT,
		D3D11_RTV_DIMENSION_TEXTURE2D,
		0,
		0
	};

	for (UINT i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		V_RETURN(pd3dDevice->CreateRenderTargetView(_shadowMapTextures[i], &shadowMapRTVDesc, &_shadowMapRTVs[i]));
	}

	// Create the shadow map depth stencil texture
	D3D11_TEXTURE2D_DESC shadowMapDSTextureDesc = 
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

	V_RETURN(pd3dDevice->CreateTexture2D(&shadowMapDSTextureDesc, NULL, &_shadowMapDSTexture));

	// create the shadow map depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowMapDSVDesc =
	{
		DXGI_FORMAT_D32_FLOAT,
		D3D11_DSV_DIMENSION_TEXTURE2D,
		0,
	};

	V_RETURN(pd3dDevice->CreateDepthStencilView(_shadowMapDSTexture, &shadowMapDSVDesc, &_shadowMapDSView));

	// Load the other IHasContents
	V_RETURN(_fsQuad.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_modelRenderer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));	

	return S_OK;
}

void DirectionalLightRenderer::OnD3D11DestroyDevice()
{
	LightRenderer::OnD3D11DestroyDevice();

	SAFE_RELEASE(_unshadowedPS);
	SAFE_RELEASE(_shadowedPS);
	SAFE_RELEASE(_lightPropertiesBuffer);
	SAFE_RELEASE(_cameraPropertiesBuffer);

	for (UINT i = 0; i < NUM_SHADOW_MAPS; i++)
	{
		SAFE_RELEASE(_shadowMapTextures[i]);
		SAFE_RELEASE(_shadowMapSRVs[i]);
		SAFE_RELEASE(_shadowMapRTVs[i]);
	}

	SAFE_RELEASE(_shadowMapDSTexture);
	SAFE_RELEASE(_shadowMapDSView);
	
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
