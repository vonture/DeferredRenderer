#include "PCH.h"
#include "TextureLoader.h"
#include "Logger.h"

template <>
HRESULT GenerateContentHash<TextureOptions>(const WCHAR* path, TextureOptions* options, long* hash)
{
	if (!hash)
	{
		return E_FAIL;
	}

	long retHash = 0;

	locale loc;
	const collate<WCHAR>& wcoll = use_facet<collate<WCHAR>>(loc);

	retHash += wcoll.hash(path, path + wcslen(path));

	*hash = retHash;
	return S_OK;
}

HRESULT Texture2DLoader::Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
	TextureOptions* options, WCHAR* errorMsg, UINT errorLen, Texture2DContent** contentOut)
{
	HRESULT hr;

	WCHAR logMsg[MAX_LOG_LENGTH];
	swprintf_s(logMsg, L"Loading texture: %s", path);
	LOG_INFO(L"Texture2DLoader", logMsg);
	
	Texture2DContent* content = new Texture2DContent();

	hr = D3DX11GetImageInfoFromFile(path, NULL, &content->Info, NULL);
	if (FAILED(hr))
	{
		FormatDXErrorMessageW(hr, errorMsg, errorLen);
		return hr;
	}

	V_RETURN(D3DX11CreateShaderResourceViewFromFile(device, path, NULL, NULL, &content->ShaderResourceView, NULL));
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