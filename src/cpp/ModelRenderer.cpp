#include "DXUT.h"
#include "ModelRenderer.h"

ModelRenderer::ModelRenderer()
	: _vertexShader(NULL), _pixelShader(NULL), _inputLayout(NULL)
{
}

ModelRenderer::~ModelRenderer()
{
}

HRESULT ModelRenderer::RenderModel(ID3D11DeviceContext* pd3dDeviceContext,ModelInstance* instance, Camera* camera)
{
	vector<ModelInstance*> v;
	v.push_back(instance);

	return RenderModels(pd3dDeviceContext, v, camera);
}

HRESULT ModelRenderer::RenderModels(ID3D11DeviceContext* pd3dDeviceContext, vector<ModelInstance*> instances, Camera* camera)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;	

	D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)DXUTGetDXGIBackBufferSurfaceDesc()->Width;
    vp.Height = (FLOAT)DXUTGetDXGIBackBufferSurfaceDesc()->Height;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;

	D3DXMATRIX view = *camera->GetView();
	//D3DXMatrixTranspose(&view, );

	D3DXMATRIX projection = *camera->GetProjection();
	//D3DXMatrixTranspose(&projection, );

	pd3dDeviceContext->GSSetShader(NULL, NULL, 0);
	pd3dDeviceContext->VSSetShader(_vertexShader, NULL, 0);
	pd3dDeviceContext->PSSetShader(_pixelShader, NULL, 0);	
	pd3dDeviceContext->IASetInputLayout(_inputLayout);
	pd3dDeviceContext->PSSetSamplers(0, 1, &_sampler);
	pd3dDeviceContext->RSSetViewports(1, &vp);

	for (vector<ModelInstance*>::iterator i = instances.begin(); i != instances.end(); i++)
	{
		D3DXMATRIX world = *(*i)->GetWorld();
		D3DXMATRIX wvp = world * view * projection;

		D3DXMatrixTranspose(&world, &world);
		D3DXMatrixTranspose(&wvp, &wvp);

		V(pd3dDeviceContext->Map(_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CB_MODEL_PROPERTIES* modelProperties = (CB_MODEL_PROPERTIES*)mappedResource.pData;
		modelProperties->World = world;
		modelProperties->WorldViewProjection = wvp;
		pd3dDeviceContext->Unmap(_constantBuffer, 0);

		pd3dDeviceContext->VSSetConstantBuffers(0, 1, &_constantBuffer);

		CDXUTSDKMesh* mesh = (*i)->GetMesh();
		mesh->Render(pd3dDeviceContext, 0, 1, 2);
	}

	return S_OK;
}

HRESULT ModelRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	ID3DBlob* pBlob = NULL;

	V_RETURN( CompileShaderFromFile( L"ModelRender.hlsl", "PS_Model", "ps_4_0", &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_pixelShader));
	SAFE_RELEASE(pBlob);	

	V_RETURN( CompileShaderFromFile( L"ModelRender.hlsl", "VS_Model", "vs_4_0", &pBlob ) );   
    V_RETURN( pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_vertexShader));
	
	const D3D11_INPUT_ELEMENT_DESC layout_mesh[] =
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	V_RETURN( pd3dDevice->CreateInputLayout(layout_mesh, ARRAYSIZE(layout_mesh), pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(), &_inputLayout));
	SAFE_RELEASE(pBlob);

	D3D11_BUFFER_DESC bufferDesc =
	{
		sizeof(CB_MODEL_PROPERTIES), //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, NULL, &_constantBuffer));

	D3D11_SAMPLER_DESC samplerDesc = 
    {
        D3D11_FILTER_MIN_MAG_MIP_LINEAR,// D3D11_FILTER Filter;
        D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressU;
        D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressV;
        D3D11_TEXTURE_ADDRESS_BORDER, //D3D11_TEXTURE_ADDRESS_MODE AddressW;
        0,//FLOAT MipLODBias;
        1,//UINT MaxAnisotropy;
        D3D11_COMPARISON_LESS , //D3D11_COMPARISON_FUNC ComparisonFunc;
        0.0,0.0,0.0,0.0,//FLOAT BorderColor[ 4 ];
        0,//FLOAT MinLOD;
        0//FLOAT MaxLOD;   
    };

	V_RETURN(pd3dDevice->CreateSamplerState(&samplerDesc, &_sampler));

	return S_OK;
}

void ModelRenderer::OnD3D11DestroyDevice()
{
	SAFE_RELEASE(_vertexShader);
	SAFE_RELEASE(_pixelShader);
	SAFE_RELEASE(_inputLayout);
	SAFE_RELEASE(_constantBuffer);
	SAFE_RELEASE(_sampler);
}

HRESULT ModelRenderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void ModelRenderer::OnD3D11ReleasingSwapChain()
{
}
