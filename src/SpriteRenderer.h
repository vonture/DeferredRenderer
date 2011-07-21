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

	static const WORD MAX_SPRITES = 4096;
	WORD _indices[MAX_SPRITES * 6];
	SPRITE_VERTEX _vertices[MAX_SPRITES * 4];
	WORD _nextSprite;

	struct TEXTURE_INDEX
	{
		WORD StartSprite;
		WORD SpriteCount;
		ID3D11ShaderResourceView* Texture;
	};
	TEXTURE_INDEX _textures[MAX_SPRITES];
	int _curTexture;

	ID3D11InputLayout* _inputLayout;
	ID3D11Buffer* _indexBuffer;
	ID3D11Buffer* _vertexBuffer;
	
	ID3D11VertexShader* _spriteVS;
	ID3D11PixelShader* _spritePS;

	ID3D11Texture2D* _blankTexture;
	ID3D11ShaderResourceView* _blankSRV;

	bool _begun;

public:
	SpriteRenderer();
	~SpriteRenderer();

	HRESULT Begin();
	HRESULT End(ID3D11DeviceContext* pd3d11DeviceContext);

	void AddTextScreenSpace(Font* font, const WCHAR* text, SPRITE_DRAW_DATA& drawData);

	void AddTexturedRectangles(ID3D11ShaderResourceView* texture, SPRITE_DRAW_DATA* spriteData,
		UINT numSprites);

	void AddColoredRectangles(SPRITE_DRAW_DATA* spriteData, UINT numSprites);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};