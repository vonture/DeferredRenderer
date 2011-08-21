#include "PCH.h"
#include "TextureLoader.h"
#include "Logger.h"
#include "DDSTextureLoader.h"

template <>
HRESULT GenerateContentHash<TextureLoadOptions>(const WCHAR* path, TextureLoadOptions* options, ContentHash* hash)
{
	if (!hash)
	{
		return FWP_E_NULL_POINTER;
	}

	ContentHash retHash = 0;

	locale loc;
	const collate<WCHAR>& wcoll = use_facet<collate<WCHAR>>(loc);

	retHash += wcoll.hash(path, path + wcslen(path));

	*hash = retHash;
	return S_OK;
}

HRESULT TextureLoader::Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
	TextureLoadOptions* options, WCHAR* errorMsg, UINT errorLen, TextureContent** contentOut)
{
	HRESULT hr;

	WCHAR logMsg[MAX_LOG_LENGTH];
	swprintf_s(logMsg, L"Loading texture: %s", path);
	LOG_INFO(L"Texture2DLoader", logMsg);
	
	TextureContent* content = new TextureContent();

	hr = D3DX11GetImageInfoFromFile(path, NULL, &content->Info, NULL);
	if (FAILED(hr))
	{
		FormatDXErrorMessageW(hr, errorMsg, errorLen);
		return hr;
	}
	
	if (options && options->Generate3DFrom2D)
	{
		hr = CreateDDSTexture3DFromFile(device, path, &content->ShaderResourceView);
	}
	else
	{
		hr = D3DX11CreateShaderResourceViewFromFile(device, path, NULL, NULL, &content->ShaderResourceView, NULL);
	}
	if (FAILED(hr))
	{
		FormatDXErrorMessageW(hr, errorMsg, errorLen);
		return hr;
	}

	if (options && options->DebugName)
	{
		SET_DEBUG_NAME(content->ShaderResourceView, options->DebugName);
	}

	*contentOut = content;
	return S_OK;
}