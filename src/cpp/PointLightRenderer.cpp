#include "PointLightRenderer.h"

PointLightRenderer::PointLightRenderer()
	: _vertexShader(NULL), _unshadowedPS(NULL), _shadowedPS(NULL), _modelPropertiesBuffer(NULL),
	  _lightPropertiesBuffer(NULL), _cameraPropertiesBuffer(NULL), _lightInputLayout(NULL),
	  _lightModel(L"\\models\\sphere\\sphere.sdkmesh")
{
}

HRESULT PointLightRenderer::RenderShadowMaps(ID3D11DeviceContext* pd3dImmediateContext, std::vector<ModelInstance*>* models,
	Camera* camera, BoundingBox* sceneBounds)
{
	return S_OK;
}

HRESULT PointLightRenderer::RenderLights(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera,
	GBuffer* gBuffer)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	
	// prepare the camera properties buffer
	XMVECTOR det;
	XMMATRIX cameraInvViewProj = XMMatrixInverse(&det, *camera->GetViewProjection());
	XMVECTOR cameraPos = camera->GetPosition();

	// Set the global properties for all point lights
	ID3D11SamplerState* samplers[2] =
	{
		GetSamplerStates()->GetLinear(),
	};
	pd3dImmediateContext->PSSetSamplers(0, 1, samplers);	

	float blendFactor[4] = {1, 1, 1, 1};
	pd3dImmediateContext->OMSetBlendState(GetBlendStates()->GetAdditiveBlend(), blendFactor, 0xFFFFFFFF);

	V_RETURN(gBuffer->PSSetShaderResources(pd3dImmediateContext, 0));	

	// Set the shaders and input
	pd3dImmediateContext->GSSetShader(NULL, NULL, 0);
	pd3dImmediateContext->VSSetShader(_vertexShader, NULL, 0);

	pd3dImmediateContext->IASetInputLayout(_lightInputLayout);

	// build the camera frustum
	BoundingFrustum cameraFrust = BoundingFrustum(*camera->GetViewProjection());

	// map the camera properties
	V_RETURN(pd3dImmediateContext->Map(_cameraPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	CB_POINTLIGHT_CAMERA_PROPERTIES* cameraProperties = (CB_POINTLIGHT_CAMERA_PROPERTIES*)mappedResource.pData;
	
	cameraProperties->InverseViewProjection = XMMatrixTranspose(cameraInvViewProj);
	XMStoreFloat4(&cameraProperties->CameraPosition, cameraPos);

	pd3dImmediateContext->Unmap(_cameraPropertiesBuffer, 0);

	pd3dImmediateContext->PSSetConstantBuffers(0, 1, &_cameraPropertiesBuffer);

	// Begin rendering unshadowed lights
	pd3dImmediateContext->PSSetShader(_unshadowedPS, NULL, 0);	

	ID3D11RasterizerState* prevRS;
	pd3dImmediateContext->RSGetState(&prevRS);

	D3D11_RASTERIZER_DESC rsDesc;
	prevRS->GetDesc(&rsDesc);

	int numUnshadowed = GetCount(false);
	for (int i = 0; i < numUnshadowed; i++)
	{
		PointLight* light = GetLight(i, false);
		XMVECTOR lightPosition = *light->GetPosition();
		float lightRadius = light->GetRadius();

		// Verify that the light is visible
		BoundingSphere lightBounds = BoundingSphere(lightPosition, lightRadius);
		if (!Intersection::Contains(&cameraFrust, &lightBounds))
		{
			continue;
		}

		// Depending on if the camera is within the light, flip the vertex winding
		if (Intersection::Contains(&lightBounds, &cameraPos))
		{
			pd3dImmediateContext->RSSetState(GetRasterizerStates()->GetFrontFaceCull());
			pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetReverseDepthEnabled(), 0);
		}
		else
		{
			pd3dImmediateContext->RSSetState(GetRasterizerStates()->GetBackFaceCull());
			pd3dImmediateContext->OMSetDepthStencilState(GetDepthStencilStates()->GetDepthEnabled(), 0);
		}
		//pd3dImmediateContext->RSSetState(GetRasterizerStates()->GetWireframe());

		// Setup the model and map the model properties
		_lightModel.SetPosition(lightPosition);
		_lightModel.SetScale(XMVectorSet(lightRadius, lightRadius, lightRadius, 1.0f));
		XMMATRIX world = *_lightModel.GetWorld();
		XMMATRIX wvp = XMMatrixMultiply(world, *camera->GetViewProjection());

		V_RETURN(pd3dImmediateContext->Map(_modelPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_POINTLIGHT_MODEL_PROPERTIES* modelProperties = 
			(CB_POINTLIGHT_MODEL_PROPERTIES*)mappedResource.pData;

		modelProperties->World = XMMatrixTranspose(world);
		modelProperties->WorldViewProjection = XMMatrixTranspose(wvp);

		pd3dImmediateContext->Unmap(_modelPropertiesBuffer, 0);

		pd3dImmediateContext->VSSetConstantBuffers(1, 1, &_modelPropertiesBuffer);

		// Map the light properties
		V_RETURN(pd3dImmediateContext->Map(_lightPropertiesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_POINTLIGHT_LIGHT_PROPERTIES* lightProperties = 
			(CB_POINTLIGHT_LIGHT_PROPERTIES*)mappedResource.pData;

		XMStoreFloat3(&lightProperties->LightColor, *light->GetColor());
		XMStoreFloat3(&lightProperties->LightPosition, lightPosition);
		lightProperties->LightIntensity = light->GetItensity();
		lightProperties->LightRadius = light->GetRadius();

		pd3dImmediateContext->Unmap(_lightPropertiesBuffer, 0);
				
		pd3dImmediateContext->PSSetConstantBuffers(2, 1, &_lightPropertiesBuffer);
		
		CDXUTSDKMesh* mesh = _lightModel.GetMesh();
		mesh->Render(pd3dImmediateContext);
	}

	// Reset the raster state
	pd3dImmediateContext->RSSetState(prevRS);
	SAFE_RELEASE(prevRS);

	return S_OK;
}

HRESULT PointLightRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	// Call base function
	V_RETURN(LightRenderer::OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	V_RETURN(_modelRenderer.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
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

	return S_OK;
}

void PointLightRenderer::OnD3D11DestroyDevice()
{
	LightRenderer::OnD3D11DestroyDevice();

	_modelRenderer.OnD3D11DestroyDevice();
	_lightModel.OnD3D11DestroyDevice();

	SAFE_RELEASE(_vertexShader);
	SAFE_RELEASE(_unshadowedPS);
	SAFE_RELEASE(_shadowedPS);
	SAFE_RELEASE(_modelPropertiesBuffer);
	SAFE_RELEASE(_lightPropertiesBuffer);
	SAFE_RELEASE(_cameraPropertiesBuffer);
	SAFE_RELEASE(_lightInputLayout);
}

HRESULT PointLightRenderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(LightRenderer::OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	
	V_RETURN(_modelRenderer.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_lightModel.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	return S_OK;
}

void PointLightRenderer::OnD3D11ReleasingSwapChain()
{
	LightRenderer::OnD3D11ReleasingSwapChain();

	_modelRenderer.OnD3D11ReleasingSwapChain();
	_lightModel.OnD3D11ReleasingSwapChain();
}