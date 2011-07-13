#include "FontRenderer.h"
#include "ShaderLoader.h"

const float FontRenderer::FONT_DEPTH = 0.5f;

FontRenderer::FontRenderer()
	: _bbWidth(1), _bbHeight(1), _nextChar(0), _inputLayout(NULL), _indexBuffer(NULL),
	  _vertexBuffer(NULL), _fontVS(NULL), _fontPS(NULL)
{
}

FontRenderer::~FontRenderer()
{
}

HRESULT FontRenderer::flush(ID3D11DeviceContext* pd3d11DeviceContext, ID3D11ShaderResourceView* fontSRV)
{
	HRESULT hr;

	if (_nextChar == 0)
	{
		return S_OK;
	}

	DXUT_BeginPerfEvent(D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f), L"Text");
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	
	UINT vbSize = sizeof(FONT_VERTEX) * _nextChar * 4;
	V_RETURN(pd3d11DeviceContext->Map(_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	memcpy(mappedResource.pData, _fontVerticies, vbSize);
	pd3d11DeviceContext->Unmap(_vertexBuffer, 0);
	
	UINT ibSize = sizeof(UINT) * _nextChar * 6;
	V_RETURN(pd3d11DeviceContext->Map(_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	memcpy(mappedResource.pData, _fontIndices, ibSize);
	pd3d11DeviceContext->Unmap(_indexBuffer, 0);

	// Set the texture
	pd3d11DeviceContext->PSSetShaderResources(0, 1, &fontSRV);

	// Set the sampler
	ID3D11SamplerState* samplers[1] = { _samplerStates.GetLinear() };
	pd3d11DeviceContext->PSSetSamplers(0, 1, samplers);

	// Set the blend state
	float blendFactor[4] = {0, 0, 0, 0};
	pd3d11DeviceContext->OMSetBlendState(_blendStates.GetAlphaBlend(), blendFactor, 0xFFFFFFFF);

	// Set the ds state
	pd3d11DeviceContext->OMSetDepthStencilState(_dsStates.GetDepthDisabled(), 0);
	
	// Set the shaders
	pd3d11DeviceContext->GSSetShader(NULL, NULL, 0);
	pd3d11DeviceContext->VSSetShader(_fontVS, NULL, 0);
	pd3d11DeviceContext->PSSetShader(_fontPS, NULL, 0);

	// Draw
    UINT stride = sizeof(FONT_VERTEX);
    UINT offset = 0;
	
	pd3d11DeviceContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	pd3d11DeviceContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, offset);
    pd3d11DeviceContext->IASetInputLayout(_inputLayout);
    pd3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3d11DeviceContext->DrawIndexed(_nextChar * 6, 0, 0);

	// Null the srv
	ID3D11ShaderResourceView* nullSRV[1] = { NULL };
	pd3d11DeviceContext->PSSetShaderResources(0, 1, nullSRV);
	
	DXUT_EndPerfEvent();

	_nextChar = 0;

	return S_OK;
}

HRESULT FontRenderer::DrawTextScreenSpace(ID3D11DeviceContext* pd3d11DeviceContext, Font* font,
		const WCHAR* text, const XMFLOAT4& color, const XMFLOAT2& topLeft, const XMFLOAT2& scale)
{
	if (!text)
	{
		return S_OK;
	}
	
	HRESULT hr;
	
	UINT lineSpacing = font->GetLineSpacing();

	UINT numChars = wcslen(text);
	XMFLOAT2 curPosition = topLeft;
	for (UINT i = 0; i < numChars; i++)
	{
		if (_nextChar >= MAX_CHARACTERS)
		{
			V_RETURN(flush(pd3d11DeviceContext, font->GetFontShaderResourceView()));
		}

		if(text[i] == '\n')
		{
			curPosition.x = topLeft.x;
			curPosition.y += lineSpacing * scale.y;
			continue;
		}

		XMFLOAT2 texCoord, texCoordSize;
		UINT charWidth;
		if(!font->GetCharacterInfo(text[i], &texCoord, &texCoordSize, &charWidth))
		{
			continue;
		}

		float pLeft = curPosition.x;
		float pTop = curPosition.y;
		float pRight = pLeft + (charWidth * scale.x);
		float pBottom = pTop + (lineSpacing * scale.y);

		// Calculate vertex position in [-1, 1] range
		pLeft = (pLeft / (_bbWidth * 0.5f)) - 1.0f;
		pTop = ((pTop / (_bbHeight * 0.5f)) - 1.0f) * -1.0f;
		pRight = (pRight / (_bbWidth * 0.5f)) - 1.0f;
		pBottom = ((pBottom / (_bbHeight * 0.5f)) - 1.0f) * -1.0f;

		float tLeft = texCoord.x;
		float tRight = texCoord.x + texCoordSize.x;
		float tTop = texCoord.y;
		float tBottom = texCoord.y + texCoordSize.y;
		
		_fontVerticies[_nextChar * 4 + 0].Position = XMFLOAT4(pLeft, pTop, FONT_DEPTH, 1.0f);
		_fontVerticies[_nextChar * 4 + 0].TexCoord = XMFLOAT2(tLeft, tTop);
		_fontVerticies[_nextChar * 4 + 0].Color = color;

		_fontVerticies[_nextChar * 4 + 1].Position = XMFLOAT4(pRight, pTop, FONT_DEPTH, 1.0f);
		_fontVerticies[_nextChar * 4 + 1].TexCoord = XMFLOAT2(tRight, tTop);
		_fontVerticies[_nextChar * 4 + 1].Color = color;

		_fontVerticies[_nextChar * 4 + 2].Position = XMFLOAT4(pLeft, pBottom, FONT_DEPTH, 1.0f);
		_fontVerticies[_nextChar * 4 + 2].TexCoord = XMFLOAT2(tLeft, tBottom);
		_fontVerticies[_nextChar * 4 + 2].Color = color;

		_fontVerticies[_nextChar * 4 + 3].Position = XMFLOAT4(pRight, pBottom, FONT_DEPTH, 1.0f);
		_fontVerticies[_nextChar * 4 + 3].TexCoord = XMFLOAT2(tRight, tBottom);
		_fontVerticies[_nextChar * 4 + 3].Color = color;

		_fontIndices[_nextChar * 6 + 0] = _nextChar * 4 + 0;
		_fontIndices[_nextChar * 6 + 1] = _nextChar * 4 + 1;
		_fontIndices[_nextChar * 6 + 2] = _nextChar * 4 + 2;

		_fontIndices[_nextChar * 6 + 3] = _nextChar * 4 + 1;
		_fontIndices[_nextChar * 6 + 4] = _nextChar * 4 + 3;
		_fontIndices[_nextChar * 6 + 5] = _nextChar * 4 + 2;

		_nextChar++;
		curPosition.x += charWidth * scale.x;
	}

	return flush(pd3d11DeviceContext, font->GetFontShaderResourceView());
}

HRESULT FontRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(_dsStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_samplerStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_blendStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_rasterStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	
	// Load the shaders
	ID3DBlob* pBlob = NULL;
	
	V_RETURN( CompileShaderFromFile( L"Font.hlsl", "PS_Font", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_fontPS));
	SAFE_RELEASE(pBlob);
	
	V_RETURN( CompileShaderFromFile( L"Font.hlsl", "VS_Font", "vs_4_0", NULL, &pBlob ) );   
    V_RETURN( pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_fontVS));

	// create the input layout
    const D3D11_INPUT_ELEMENT_DESC fontlayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    V_RETURN( pd3dDevice->CreateInputLayout(fontlayout, 3, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &_inputLayout));
    SAFE_RELEASE( pBlob );

	D3D11_BUFFER_DESC vbDesc = 
	{
		sizeof(FONT_VERTEX) * MAX_CHARACTERS * 6, // INT ByteWidth;
		D3D11_USAGE_DYNAMIC, // D3D11_USAGE Usage;
		D3D11_BIND_VERTEX_BUFFER, // UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, // UINT CPUAccessFlags;
		0, // UINT MiscFlags;
		0, // UINT StructureByteStride;
	};
	V_RETURN(pd3dDevice->CreateBuffer(&vbDesc, NULL, &_vertexBuffer));

	D3D11_BUFFER_DESC ibDesc = 
	{
		sizeof(UINT) * MAX_CHARACTERS * 4, // INT ByteWidth;
		D3D11_USAGE_DYNAMIC, // D3D11_USAGE Usage;
		D3D11_BIND_INDEX_BUFFER, // UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, // UINT CPUAccessFlags;
		0, // UINT MiscFlags;
		0, // UINT StructureByteStride;
	};
	V_RETURN(pd3dDevice->CreateBuffer(&ibDesc, NULL, &_indexBuffer));

	// Store backbuffer size
	_bbWidth = pBackBufferSurfaceDesc->Width;
	_bbHeight = pBackBufferSurfaceDesc->Height;

	return S_OK; 
}

void FontRenderer::OnD3D11DestroyDevice()
{
	_dsStates.OnD3D11DestroyDevice();
	_samplerStates.OnD3D11DestroyDevice();
	_blendStates.OnD3D11DestroyDevice();
	_rasterStates.OnD3D11DestroyDevice();
	
	SAFE_RELEASE(_inputLayout);
	SAFE_RELEASE(_indexBuffer);
	SAFE_RELEASE(_vertexBuffer);
	SAFE_RELEASE(_fontVS);
	SAFE_RELEASE(_fontPS);
}

HRESULT FontRenderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	
	V_RETURN(_dsStates.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_samplerStates.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_blendStates.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));
	V_RETURN(_rasterStates.OnD3D11ResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc));

	// Store backbuffer size
	_bbWidth = pBackBufferSurfaceDesc->Width;
	_bbHeight = pBackBufferSurfaceDesc->Height;

	return S_OK;
}

void FontRenderer::OnD3D11ReleasingSwapChain()
{
	_dsStates.OnD3D11ReleasingSwapChain();
	_samplerStates.OnD3D11ReleasingSwapChain();
	_blendStates.OnD3D11ReleasingSwapChain();
	_rasterStates.OnD3D11ReleasingSwapChain();
}