#pragma once

#include "PCH.h"
#include "IHasContent.h"
#include "ContentType.h"

class SpriteFont : public ContentType
{
private:
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
	SpriteFont();
	~SpriteFont();

	bool ContainsCharacter(WCHAR character);
	
	XMFLOAT2 MeasureString(const WCHAR* text);

	bool GetCharacterInfo(WCHAR character, XMFLOAT2* outTexCoord, XMFLOAT2* outTexCoordSize, UINT* outWidth);
	UINT GetLineSpacing();

	ID3D11ShaderResourceView* GetFontShaderResourceView();

	HRESULT CreateFromFile(ID3D11Device* pd3dDevice, const WCHAR* path);
	void Destroy();
};