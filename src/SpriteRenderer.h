#pragma once

#include "Defines.h"
#include "IHasContent.h"
#include "DeviceStates.h"
#include "Font.h"

struct SPRITE_DRAW_DATA
{
	XMFLOAT2 TopLeft;
	XMFLOAT2 Size;	
	XMFLOAT2 TopLeftUV;
	XMFLOAT2 SizeUV;	
	XMFLOAT4 Color;
};

class SpriteRenderer : public IHasContent
{
private:
	DepthStencilStates _dsStates;
	SamplerStates _samplerStates;
	BlendStates _blendStates;
	RasterizerStates _rasterStates;

	UINT _bbWidth;
	UINT _bbHeight;

	static const float SPRITE_DEPTH;

	struct SPRITE_VERTEX
	{
		XMFLOAT4 Position;
		XMFLOAT2 TexCoord;
		XMFLOAT4 Color;
	};

	static const WORD MAX_SPRITES = 256;
	WORD _indices[MAX_SPRITES * 6];
	SPRITE_VERTEX _vertices[MAX_SPRITES * 4];
	WORD _nextSprite;

	ID3D11InputLayout* _inputLayout;
	ID3D11Buffer* _indexBuffer;
	ID3D11Buffer* _vertexBuffer;
	
	ID3D11VertexShader* _spriteVS;
	ID3D11PixelShader* _spritePS;

	ID3D11Texture2D* _blankTexture;
	ID3D11ShaderResourceView* _blankSRV;

	HRESULT flush(ID3D11DeviceContext* pd3d11DeviceContext, ID3D11ShaderResourceView* srv);

public:
	SpriteRenderer();
	~SpriteRenderer();

	HRESULT DrawTextScreenSpace(ID3D11DeviceContext* pd3d11DeviceContext, Font* font,
		const WCHAR* text, SPRITE_DRAW_DATA& drawData);

	HRESULT DrawTexturedRectangles(ID3D11DeviceContext* pd3d11DeviceContext, ID3D11ShaderResourceView* texture,
		SPRITE_DRAW_DATA* spriteData, UINT numSprites);

	HRESULT DrawColoredRectangles(ID3D11DeviceContext* pd3d11DeviceContext, SPRITE_DRAW_DATA* spriteData,
		UINT numSprites);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};