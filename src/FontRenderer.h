#pragma once

#include "Defines.h"
#include "IHasContent.h"
#include "DeviceStates.h"
#include "Camera.h"

class FontRenderer : public IHasContent
{
private:
	DepthStencilStates _dsStates;
	SamplerStates _samplerStates;
	BlendStates _blendStates;
	RasterizerStates _rasterStates;

	bool _begun;
	UINT _bbWidth;
	UINT _bbHeight;
	
	static const UINT FIRST_CHAR = 32;
	static const UINT LAST_CHAR = 126;
	static const UINT GLYPH_WIDTH = 15;
	static const UINT GLYPH_HEIGHT = 42;
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

	const WCHAR* _fontPath;
	ID3D11ShaderResourceView* _fontSRV;

	ID3D11VertexShader* _fontVS;
	ID3D11PixelShader* _fontPS;

public:
	FontRenderer(const WCHAR* fontPath);
	~FontRenderer();

	HRESULT Begin();
	HRESULT End(ID3D11DeviceContext* pd3d11DeviceContext);

	void AddTextScreenSpace(const WCHAR* text, const XMFLOAT4& color, const XMFLOAT2& topLeft,
		const XMFLOAT2& scale);
	XMFLOAT2 MeasureTextScreenSpace(const WCHAR* text);

	void AddTextWorldSpaceBillboard(const WCHAR* text, const XMFLOAT4& color, const XMFLOAT3& position,
		const XMFLOAT2& scale, Camera* camera);

	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	virtual void OnD3D11DestroyDevice();

	virtual HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	virtual void OnD3D11ReleasingSwapChain();
};