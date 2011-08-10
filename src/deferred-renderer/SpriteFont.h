#pragma once

#include "PCH.h"
#include "IHasContent.h"

class SpriteFont : public IHasContent
{
private:
	const WCHAR* _path;

	struct CharInfo
	{		
		float X, Y, Width, Height;
		UINT PixelWidth;
	};

	UINT _lineSpacing;

	std::map<UINT, CharInfo> _charMap;

	int _textureWidth;
	int _textureHeight;

	ID3D11ShaderResourceView* _fontSRV;

public:
	SpriteFont(const WCHAR* path);
	~SpriteFont();

	bool ContainsCharacter(WCHAR character);
	
	XMFLOAT2 MeasureString(const WCHAR* text);

	bool GetCharacterInfo(WCHAR character, XMFLOAT2* outTexCoord, XMFLOAT2* outTexCoordSize, UINT* outWidth);
	UINT GetLineSpacing();

	ID3D11ShaderResourceView* GetFontShaderResourceView();

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};