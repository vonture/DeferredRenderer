#include "PCH.h"
#include "TextureLoader.h"
#include "Logger.h"
#include "DDSTextureLoader.h"

HRESULT TextureLoader::GenerateContentHash(const WCHAR* path, TextureOptions* options, ContentHash* hash)
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

HRESULT TextureLoader::LoadFromContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
	TextureOptions* options, WCHAR* errorMsg, UINT errorLen, TextureContent** contentOut)
{
	HRESULT hr;

	WCHAR logMsg[MAX_LOG_LENGTH];
	swprintf_s(logMsg, L"Loading - %s", path);
	LOG_INFO(L"Texture Loader", logMsg);
	
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
		CHAR debugName[256];

		sprintf_s(debugName, "%s %s", options->DebugName, "SRV");
		SET_DEBUG_NAME(content->ShaderResourceView, debugName);
	}

	*contentOut = content;
	return S_OK;
}

HRESULT TextureLoader::LoadFromCompiledContentFile(ID3D11Device* device, const WCHAR* path, WCHAR* errorMsg,
		UINT errorLen, TextureContent** contentOut)
{
	return E_NOTIMPL;
}