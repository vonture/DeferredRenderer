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

	// Load the fullscreen quad
	V_RETURN(_fsQuad.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	
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
}

HRESULT DirectionalLightRenderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(LightRenderer::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_fsQuad.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void DirectionalLightRenderer::OnD3D11ReleasingSwapChain()
{
	LightRenderer::OnD3D11ReleasingSwapChain();
	_fsQuad.OnD3D11ReleasingSwapChain();
}
