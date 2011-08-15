#include "PCH.h"
#include "SpriteFont.h"
#include "tinyxml.h"

SpriteFont::SpriteFont(const WCHAR* path)
	: _path(path), _textureWidth(1), _textureHeight(1), _fontSRV(NULL)
{
}

SpriteFont::~SpriteFont()
{
}

bool SpriteFont::ContainsCharacter(WCHAR character)
{
	return _charMap.find(character) != _charMap.end();
}
	
XMFLOAT2 SpriteFont::MeasureString(const WCHAR* text)
{
	UINT numChars = (int)wcslen(text);

	XMFLOAT2 size = XMFLOAT2(0.0f, (float)_lineSpacing);
	float rowLen = 0.0f;
	for (UINT i = 0; i < numChars; i++)
	{
		if(text[i] == '\n')
		{
			size.y += _lineSpacing;
			rowLen = 0.0f;
		}
		else if (!ContainsCharacter(text[i]))
		{
			continue;
		}
		
		rowLen += _charMap[text[i]].PixelWidth;
		size.x = max(size.x, rowLen);
	}

	return size;
}

bool SpriteFont::GetCharacterInfo(WCHAR character, XMFLOAT2* outTexCoord, XMFLOAT2* outTexCoordSize, UINT* outWidth)
{
	if (!ContainsCharacter(character))
	{
		return false;
	}

	CharInfo info = _charMap[character];
	*outTexCoord = XMFLOAT2(info.X, info.Y);
	*outTexCoordSize = XMFLOAT2(info.Width, info.Height);
	*outWidth = info.PixelWidth;

	return true;
}

UINT SpriteFont::GetLineSpacing()
{
	return _lineSpacing;
}

ID3D11ShaderResourceView* SpriteFont::GetFontShaderResourceView()
{
	return _fontSRV;
}

HRESULT SpriteFont::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	WCHAR fullFontPath[MAX_PATH];
	V_RETURN(DXUTFindDXSDKMediaFileCch(fullFontPath, MAX_PATH, _path));	

	char sPath[MAX_PATH];
	if (!WStringToAnsi(fullFontPath, sPath, MAX_PATH))
	{
		return E_FAIL;
	}

	TiXmlDocument doc = TiXmlDocument(sPath);
	if (!doc.LoadFile())
	{
		return E_FAIL;
	}
	
	TiXmlElement* root = doc.FirstChildElement("fontMetrics");
	if (!root)
	{
		return E_FAIL;
	}
	
	const char* texName = root->Attribute("file");
	if (!texName)
	{
		return E_FAIL;
	}

	WCHAR wTextureName[MAX_PATH];
	if (!AnsiToWString(texName, wTextureName, MAX_PATH))
	{
		return E_FAIL;
	}

	WCHAR wDirName[MAX_PATH];
	if (!GetDirectoryFromFileNameW(fullFontPath, wDirName, MAX_PATH))
	{
		return E_FAIL;
	}

	WCHAR fullPath[MAX_PATH];
	wcsncpy_s(fullPath, wDirName, MAX_PATH);
	wcsncat_s(fullPath, L"\\", MAX_PATH);
	wcsncat_s(fullPath, wTextureName, MAX_PATH);

	V_RETURN(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, fullPath, NULL, NULL, &_fontSRV, NULL));
	char debugName[MAX_PATH];
	sprintf_s(debugName, "%s font SRV", texName);
	SET_DEBUG_NAME(_fontSRV, debugName);

	D3DX11_IMAGE_INFO info;
	V_RETURN(D3DX11GetImageInfoFromFile(fullPath, NULL, &info, NULL));
	_textureWidth = info.Width;
	_textureHeight = info.Height;

	TiXmlElement* child = root->FirstChildElement("character");
	while (child)
	{
		int character;
		if (!child->Attribute("key", &character))
		{
			return E_FAIL;
		}

		int x, y, width, height;
		
		TiXmlElement* element = child->FirstChildElement("x");
		if (!element)
		{
			return E_FAIL;
		}
		x = atoi(element->GetText());
		
		element = child->FirstChildElement("y");
		if (!element)
		{
			return E_FAIL;
		}
		y = atoi(element->GetText());

		element = child->FirstChildElement("width");
		if (!element)
		{
			return E_FAIL;
		}
		width = atoi(element->GetText());

		element = child->FirstChildElement("height");
		if (!element)
		{
			return E_FAIL;
		}
		height = atoi(element->GetText());

		CharInfo info;
		info.X = x / (float)_textureWidth;
		info.Y = y / (float)_textureHeight;
		info.Width = width / (float)_textureWidth;
		info.Height = height / (float)_textureHeight;
		info.PixelWidth = width;		
		_lineSpacing = height;

		_charMap[(UINT)character] = info;
		
		child = child->NextSiblingElement("character");
	}

	return S_OK;
}

void SpriteFont::OnD3D11DestroyDevice()
{
	SAFE_RELEASE(_fontSRV);
	_charMap.clear();
}

HRESULT SpriteFont::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void SpriteFont::OnD3D11ReleasingSwapChain()
{
}