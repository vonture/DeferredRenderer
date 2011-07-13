#pragma once

#include "Defines.h"
#include "IHasContent.h"
#include "DeviceStates.h"
#include "Font.h"

class FontRenderer : public IHasContent
{
private:
	DepthStencilStates _dsStates;
	SamplerStates _samplerStates;
	BlendStates _blendStates;
	RasterizerStates _rasterStates;

	UINT _bbWidth;
	UINT _bbHeight;
	
	static const float FONT_DEPTH;

	struct FONT_VERTEX
	{
		XMFLOAT4 Position;
		XMFLOAT2 TexCoord;
		XMFLOAT4 Color;
	};

	static const UINT MAX_CHARACTERS = 4096;
	UINT _fontIndices[MAX_CHARACTERS * 6];
	FONT_VERTEX _fontVerticies[MAX_CHARACTERS * 4];
	UINT _nextChar;
	
	ID3D11InputLayout* _inputLayout;
	ID3D11Buffer* _indexBuffer;
	ID3D11Buffer* _vertexBuffer;
	
	ID3D11VertexShader* _fontVS;
	ID3D11PixelShader* _fontPS;

	HRESULT flush(ID3D11DeviceContext* pd3d11DeviceContext, ID3D11ShaderResourceView* fontSRV);

public:
	FontRenderer();
	~FontRenderer();
	
	HRESULT DrawTextScreenSpace(ID3D11DeviceContext* pd3d11DeviceContext, Font* font,
		const WCHAR* text, const XMFLOAT4& color, const XMFLOAT2& topLeft, const XMFLOAT2& scale);

	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	virtual void OnD3D11DestroyDevice();

	virtual HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	virtual void OnD3D11ReleasingSwapChain();
};