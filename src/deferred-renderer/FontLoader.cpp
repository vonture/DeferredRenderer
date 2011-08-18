#include "PCH.h"
#include "FontLoader.h"

template <>
HRESULT GenerateContentHash<FontOptions>(const WCHAR* path, FontOptions* options, long* hash)
{
	if (!hash)
	{
		return E_FAIL;
	}

	locale loc;
	const collate<WCHAR>& wcoll = use_facet<collate<WCHAR>>(loc);

	*hash = wcoll.hash(path, path + wcslen(path));
	return S_OK;
}

HRESULT FontLoader::Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		FontOptions* options, WCHAR* errorMsg, UINT errorLen, SpriteFont** contentOut)
{
	WCHAR logMsg[MAX_LOG_LENGTH];
	swprintf_s(logMsg, L"Loading - %s", path);
	LOG_INFO(L"Font Loader", logMsg);

	SpriteFont* font = new SpriteFont();

	HRESULT hr = font->CreateFromFile(device, path);
	if (FAILED(hr))
	{
		delete font;
		return hr;
	}

	*contentOut = font;
	return S_OK;
}