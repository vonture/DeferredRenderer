#include "PCH.h"
#include "Quad.h"
#include "VertexShaderLoader.h"

Quad::Quad() 
	: _vertexShader(NULL), _inputLayout(NULL), _vertexBuffer(NULL)
{
}

Quad::~Quad() 
{
}

HRESULT Quad::Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11PixelShader* pixelShader)
{
    UINT strides = sizeof(QUAD_VERTEX);
    UINT offsets = 0;
    ID3D11Buffer* pBuffers[1] = { _vertexBuffer };

    pd3dImmediateContext->IASetInputLayout(_inputLayout);
    pd3dImmediateContext->IASetVertexBuffers(0, 1, pBuffers, &strides, &offsets);
    pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    pd3dImmediateContext->VSSetShader(_vertexShader, NULL, 0);
    pd3dImmediateContext->PSSetShader(pixelShader, NULL, 0);
    pd3dImmediateContext->Draw(4, 0);

	return S_OK;
}

HRESULT Quad::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	// Build the vertex buffer with initial data created by the fillVerticies method
	QUAD_VERTEX verts[4];
	fillVerticies(&verts[0]);

	D3D11_BUFFER_DESC vbDesc =
    {
        4 * sizeof(QUAD_VERTEX),
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_VERTEX_BUFFER,
        0,
        0
    };

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = verts;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;

    V_RETURN(pd3dDevice->CreateBuffer(&vbDesc, &initData, &_vertexBuffer));

	// Create the vertex shader
	D3D11_INPUT_ELEMENT_DESC quadlayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0,       DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

	VertexShaderOptions vsOpts;
	vsOpts.EntryPoint = "VS_Quad";
	vsOpts.Defines = NULL;
	vsOpts.InputElements = quadlayout;
	vsOpts.InputElementCount = ARRAYSIZE(quadlayout);
	vsOpts.DebugName = "Quad";

	VertexShaderContent* vsContent;
	V_RETURN(pContentManager->LoadContent(pd3dDevice, L"Quad.hlsl", &vsOpts, &vsContent));

	_vertexShader = vsContent->VertexShader;
	_inputLayout = vsContent->InputLayout;

	_vertexShader->AddRef();
	_inputLayout->AddRef();

	SAFE_RELEASE(vsContent);

	return S_OK;
}

void Quad::OnD3D11DestroyDevice()
{
	SAFE_RELEASE(_vertexShader);
	SAFE_RELEASE(_inputLayout);
	SAFE_RELEASE(_vertexBuffer);
}

HRESULT Quad::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void Quad::OnD3D11ReleasingSwapChain()
{
}