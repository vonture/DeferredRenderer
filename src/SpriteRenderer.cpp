#include "SpriteRenderer.h"
#include "ShaderLoader.h"

const float SpriteRenderer::SPRITE_DEPTH = 0.5f;

SpriteRenderer::SpriteRenderer()
	: _bbWidth(1), _bbHeight(1), _nextSprite(0), _inputLayout(NULL), _indexBuffer(NULL),
	  _vertexBuffer(NULL), _spriteVS(NULL), _spritePS(NULL), _blankTexture(NULL), _blankSRV(NULL),
	  _begun(false)
{
}

SpriteRenderer::~SpriteRenderer()
{
}

HRESULT SpriteRenderer::Begin()
{
	if (_begun)
	{
		return E_FAIL;
	}
	_begun = true;

	_nextSprite = 0;
	_curTexture = -1;

	return S_OK;
}

HRESULT SpriteRenderer::End(ID3D11DeviceContext* pd3d11DeviceContext)
{
	HRESULT hr;

	if (!_begun)
	{
		return E_FAIL;
	}
	_begun = false;

	if (_nextSprite == 0)
	{
		return S_OK;
	}

	D3DPERF_BeginEvent(D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f), L"Sprite");
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	
	// Map the vertex buffer
	UINT vbSize = sizeof(SPRITE_VERTEX) * _nextSprite * 4;
	V_RETURN(pd3d11DeviceContext->Map(_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	memcpy(mappedResource.pData, _vertices, vbSize);
	pd3d11DeviceContext->Unmap(_vertexBuffer, 0);
	
	// Map the index buffer
	UINT ibSize = sizeof(WORD) * _nextSprite * 6;
	V_RETURN(pd3d11DeviceContext->Map(_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	memcpy(mappedResource.pData, _indices, ibSize);
	pd3d11DeviceContext->Unmap(_indexBuffer, 0);

	// Set the sampler
	ID3D11SamplerState* samplers[1] = { _samplerStates.GetLinearClamp() };
	pd3d11DeviceContext->PSSetSamplers(0, 1, samplers);

	// Set the blend state
	float blendFactor[4] = {0, 0, 0, 0};
	pd3d11DeviceContext->OMSetBlendState(_blendStates.GetAlphaBlend(), blendFactor, 0xFFFFFFFF);

	// Set the ds state
	pd3d11DeviceContext->OMSetDepthStencilState(_dsStates.GetDepthDisabled(), 0);
	
	// Set the shaders
	pd3d11DeviceContext->GSSetShader(NULL, NULL, 0);
	pd3d11DeviceContext->VSSetShader(_spriteVS, NULL, 0);
	pd3d11DeviceContext->PSSetShader(_spritePS, NULL, 0);

	// Set the buffers
	UINT stride = sizeof(SPRITE_VERTEX);
    UINT offset = 0;
	
	pd3d11DeviceContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	pd3d11DeviceContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R16_UINT, offset);
    pd3d11DeviceContext->IASetInputLayout(_inputLayout);
    pd3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (int i = 0; i <= _curTexture; i++)
	{
		// Set the texture
		pd3d11DeviceContext->PSSetShaderResources(0, 1, &_textures[i].Texture);

		// Draw
		pd3d11DeviceContext->DrawIndexed(_textures[i].SpriteCount * 6, _textures[i].StartSprite * 6, 0);
	}

	// Null the srv
	ID3D11ShaderResourceView* nullSRV[1] = { NULL };
	pd3d11DeviceContext->PSSetShaderResources(0, 1, nullSRV);

	D3DPERF_EndEvent();

	return S_OK;
}

void SpriteRenderer::AddTextScreenSpace(Font* font, const WCHAR* text, SPRITE_DRAW_DATA& drawData)
{
	if (_nextSprite >= MAX_SPRITES)
	{
		return;
	}

	if (!text)
	{
		return;
	}

	XMFLOAT2 textDrawSize = font->MeasureString(text);
	if (textDrawSize.x <= 0.0f || textDrawSize.y <= 0.0f)
	{
		return;
	}
		
	ID3D11ShaderResourceView* fontSRV = font->GetFontShaderResourceView();
	if (_curTexture < 0 || _textures[_curTexture].Texture != fontSRV)
	{
		_curTexture++;
		_textures[_curTexture].StartSprite = _nextSprite;
		_textures[_curTexture].SpriteCount = 0;
		_textures[_curTexture].Texture = fontSRV;
	}

	UINT lineSpacing = font->GetLineSpacing();	

	XMFLOAT2 scale = XMFLOAT2(drawData.Size.x / textDrawSize.x, drawData.Size.y / textDrawSize.y);

	UINT numChars = wcslen(text);
	XMFLOAT2 curPosition = drawData.TopLeft;
	for (UINT i = 0; i < numChars && _nextSprite < MAX_SPRITES; i++)
	{
		if(text[i] == '\n')
		{
			curPosition.x = drawData.TopLeft.x;
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
		
		_vertices[_nextSprite * 4 + 0].Position = XMFLOAT4(pLeft, pTop, SPRITE_DEPTH, 1.0f);
		_vertices[_nextSprite * 4 + 0].TexCoord = XMFLOAT2(tLeft, tTop);
		_vertices[_nextSprite * 4 + 0].Color = drawData.Color;

		_vertices[_nextSprite * 4 + 1].Position = XMFLOAT4(pRight, pTop, SPRITE_DEPTH, 1.0f);
		_vertices[_nextSprite * 4 + 1].TexCoord = XMFLOAT2(tRight, tTop);
		_vertices[_nextSprite * 4 + 1].Color = drawData.Color;

		_vertices[_nextSprite * 4 + 2].Position = XMFLOAT4(pLeft, pBottom, SPRITE_DEPTH, 1.0f);
		_vertices[_nextSprite * 4 + 2].TexCoord = XMFLOAT2(tLeft, tBottom);
		_vertices[_nextSprite * 4 + 2].Color = drawData.Color;
		
		_vertices[_nextSprite * 4 + 3].Position = XMFLOAT4(pRight, pBottom, SPRITE_DEPTH, 1.0f);
		_vertices[_nextSprite * 4 + 3].TexCoord = XMFLOAT2(tRight, tBottom);
		_vertices[_nextSprite * 4 + 3].Color = drawData.Color;

		_indices[_nextSprite * 6 + 0] = _nextSprite * 4 + 0;
		_indices[_nextSprite * 6 + 1] = _nextSprite * 4 + 1;
		_indices[_nextSprite * 6 + 2] = _nextSprite * 4 + 2;

		_indices[_nextSprite * 6 + 3] = _nextSprite * 4 + 1;
		_indices[_nextSprite * 6 + 4] = _nextSprite * 4 + 3;
		_indices[_nextSprite * 6 + 5] = _nextSprite * 4 + 2;
		
		curPosition.x += charWidth * scale.x;

		_nextSprite++;
		_textures[_curTexture].SpriteCount++;
	}
}

void SpriteRenderer::AddTexturedRectangles( ID3D11ShaderResourceView* texture, SPRITE_DRAW_DATA* spriteData,
	UINT numSprites)
{
	if (_nextSprite >= MAX_SPRITES)
	{
		return;
	}

	if (_curTexture < 0 || _textures[_curTexture].Texture != texture)
	{
		_curTexture++;
		_textures[_curTexture].StartSprite = _nextSprite;
		_textures[_curTexture].SpriteCount = 0;
		_textures[_curTexture].Texture = texture;
	}

	for (WORD i = 0; i < numSprites && _nextSprite < MAX_SPRITES; i++)
	{
		SPRITE_DRAW_DATA& data = spriteData[i];

		float pLeft = (data.TopLeft.x / (_bbWidth * 0.5f)) - 1.0f;
		float pTop = ((data.TopLeft.y / (_bbHeight * 0.5f)) - 1.0f) * -1.0f;
		float pRight = ((data.TopLeft.x + data.Size.x) / (_bbWidth * 0.5f)) - 1.0f;
		float pBottom = (((data.TopLeft.y + data.Size.y) / (_bbHeight * 0.5f)) - 1.0f) * -1.0f;

		float tLeft = data.TopLeftUV.x;
		float tRight = data.TopLeftUV.x + data.SizeUV.x;
		float tTop = data.TopLeftUV.y;
		float tBottom = data.TopLeftUV.y + data.SizeUV.y;

		_vertices[_nextSprite * 4 + 0].Position = XMFLOAT4(pLeft, pTop, SPRITE_DEPTH, 1.0f);
		_vertices[_nextSprite * 4 + 0].TexCoord = XMFLOAT2(tLeft, tTop);
		_vertices[_nextSprite * 4 + 0].Color = data.Color;

		_vertices[_nextSprite * 4 + 1].Position = XMFLOAT4(pRight, pTop, SPRITE_DEPTH, 1.0f);
		_vertices[_nextSprite * 4 + 1].TexCoord = XMFLOAT2(tRight, tTop);
		_vertices[_nextSprite * 4 + 1].Color = data.Color;

		_vertices[_nextSprite * 4 + 2].Position = XMFLOAT4(pLeft, pBottom, SPRITE_DEPTH, 1.0f);
		_vertices[_nextSprite * 4 + 2].TexCoord = XMFLOAT2(tLeft, tBottom);
		_vertices[_nextSprite * 4 + 2].Color = data.Color;
		
		_vertices[_nextSprite * 4 + 3].Position = XMFLOAT4(pRight, pBottom, SPRITE_DEPTH, 1.0f);
		_vertices[_nextSprite * 4 + 3].TexCoord = XMFLOAT2(tRight, tBottom);
		_vertices[_nextSprite * 4 + 3].Color = data.Color;

		_indices[_nextSprite * 6 + 0] = _nextSprite * 4 + 0;
		_indices[_nextSprite * 6 + 1] = _nextSprite * 4 + 1;
		_indices[_nextSprite * 6 + 2] = _nextSprite * 4 + 2;

		_indices[_nextSprite * 6 + 3] = _nextSprite * 4 + 1;
		_indices[_nextSprite * 6 + 4] = _nextSprite * 4 + 3;
		_indices[_nextSprite * 6 + 5] = _nextSprite * 4 + 2;

		_nextSprite++;
		_textures[_curTexture].SpriteCount++;
	}
}

void SpriteRenderer::AddColoredRectangles(SPRITE_DRAW_DATA* spriteData, UINT numSprites)
{
	AddTexturedRectangles(_blankSRV, spriteData, numSprites);
}

HRESULT SpriteRenderer::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN(_dsStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_samplerStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_blendStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	V_RETURN(_rasterStates.OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc));

	// Load the shaders
	ID3DBlob* pBlob = NULL;
	
	V_RETURN(CompileShaderFromFile( L"Sprite.hlsl", "PS_Sprite", "ps_4_0", NULL, &pBlob ) );   
    V_RETURN(pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_spritePS));
	SAFE_RELEASE(pBlob);
	SET_DEBUG_NAME(_spritePS, "Sprite renderer PS");
	
	V_RETURN(CompileShaderFromFile(L"Sprite.hlsl", "VS_Sprite", "vs_4_0", NULL, &pBlob));   
    V_RETURN(pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_spriteVS));
	SET_DEBUG_NAME(_spriteVS, "Sprite renderer VS");

	// create the input layout
    const D3D11_INPUT_ELEMENT_DESC fontlayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    V_RETURN(pd3dDevice->CreateInputLayout(fontlayout, 3, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &_inputLayout));
    SAFE_RELEASE(pBlob);
	SET_DEBUG_NAME(_spriteVS, "Sprite renderer input layout");

	D3D11_BUFFER_DESC vbDesc = 
	{
		sizeof(SPRITE_VERTEX) * MAX_SPRITES * 6, // INT ByteWidth;
		D3D11_USAGE_DYNAMIC, // D3D11_USAGE Usage;
		D3D11_BIND_VERTEX_BUFFER, // UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, // UINT CPUAccessFlags;
		0, // UINT MiscFlags;
		0, // UINT StructureByteStride;
	};
	V_RETURN(pd3dDevice->CreateBuffer(&vbDesc, NULL, &_vertexBuffer));
	SET_DEBUG_NAME(_vertexBuffer, "Sprite renderer VB");

	D3D11_BUFFER_DESC ibDesc = 
	{
		sizeof(WORD) * MAX_SPRITES * 4, // INT ByteWidth;
		D3D11_USAGE_DYNAMIC, // D3D11_USAGE Usage;
		D3D11_BIND_INDEX_BUFFER, // UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, // UINT CPUAccessFlags;
		0, // UINT MiscFlags;
		0, // UINT StructureByteStride;
	};
	V_RETURN(pd3dDevice->CreateBuffer(&ibDesc, NULL, &_indexBuffer));
	SET_DEBUG_NAME(_indexBuffer, "Sprite renderer IB");

	// create the blank texture
	D3D11_TEXTURE2D_DESC blankTextureDesc = 
    {
        1,//UINT Width;
        1,//UINT Height;
        1,//UINT MipLevels;
        1,//UINT ArraySize;
        DXGI_FORMAT_R16G16B16A16_FLOAT,//DXGI_FORMAT Format;
        1,//DXGI_SAMPLE_DESC SampleDesc;
        0,
        D3D11_USAGE_DEFAULT,//D3D11_USAGE Usage;
        D3D11_BIND_SHADER_RESOURCE,//UINT BindFlags;
        0,//UINT CPUAccessFlags;
        0//UINT MiscFlags;    
    };
		
	XMHALF4 initPixel = XMHALF4(1.0f, 1.0f, 1.0f, 1.0f);

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &initPixel;
	initData.SysMemPitch = sizeof(XMHALF4);
	initData.SysMemSlicePitch = 0;

	V_RETURN(pd3dDevice->CreateTexture2D(&blankTextureDesc, &initData, &_blankTexture));
	SET_DEBUG_NAME(_blankTexture, "Sprite renderer blank texture");

	// create the blank srv
	D3D11_SHADER_RESOURCE_VIEW_DESC blankSRVDesc = 
    {
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        D3D11_SRV_DIMENSION_TEXTURE2D,
        0,
        0
    };
	blankSRVDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(_blankTexture, &blankSRVDesc, &_blankSRV));
	SET_DEBUG_NAME(_blankTexture, "Sprite renderer blank SRV");
	
	// Store backbuffer size
	_bbWidth = pBackBufferSurfaceDesc->Width;
	_bbHeight = pBackBufferSurfaceDesc->Height;
	
	return S_OK; 
}

void SpriteRenderer::OnD3D11DestroyDevice()
{
	_dsStates.OnD3D11DestroyDevice();
	_samplerStates.OnD3D11DestroyDevice();
	_blendStates.OnD3D11DestroyDevice();
	_rasterStates.OnD3D11DestroyDevice();
	
	SAFE_RELEASE(_inputLayout);
	SAFE_RELEASE(_indexBuffer);
	SAFE_RELEASE(_vertexBuffer);
	SAFE_RELEASE(_spriteVS);
	SAFE_RELEASE(_spritePS);
	SAFE_RELEASE(_blankTexture);
	SAFE_RELEASE(_blankSRV);
}

HRESULT SpriteRenderer::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
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

void SpriteRenderer::OnD3D11ReleasingSwapChain()
{
	_dsStates.OnD3D11ReleasingSwapChain();
	_samplerStates.OnD3D11ReleasingSwapChain();
	_blendStates.OnD3D11ReleasingSwapChain();
	_rasterStates.OnD3D11ReleasingSwapChain();
}