#include "PCH.h"
#include "PixelShaderLoader.h"
#include "Logger.h"

template <>
HRESULT GenerateContentHash<PixelShaderOptions>(const WCHAR* path, PixelShaderOptions* options, long* hash)
{
	if (!hash)
	{
		return E_FAIL;
	}

	long retHash = 0;

	locale loc;
	const collate<WCHAR>& wcoll = use_facet<collate<WCHAR>>(loc);

	retHash += wcoll.hash(path, path + wcslen(path));

	if (options)
	{
		const collate<char>& acoll = use_facet<collate<char>>(loc);

		UINT defineIdx = 0;
		while (options->Defines[defineIdx].Name)
		{
			retHash += acoll.hash(options->Defines[defineIdx].Name,
				options->Defines[defineIdx].Name + strlen(options->Defines[defineIdx].Name));

			retHash += acoll.hash(options->Defines[defineIdx].Definition,
				options->Defines[defineIdx].Definition + strlen(options->Defines[defineIdx].Definition));

			defineIdx++;
		}

		// Not hashing the debug name since it does not affect the content that is loaded
	}

	*hash = retHash;
	return S_OK;
}

HRESULT PixelShaderLoader::Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
	PixelShaderOptions* options, WCHAR* errorMsg, UINT errorLen, PixelShaderContent** contentOut)
{
	if (!options)
	{
		swprintf_s(errorMsg, errorLen, L"Options cannot be null when loading shaders.");
		return E_FAIL;
	}
	
	WCHAR logMsg[MAX_LOG_LENGTH];
	swprintf_s(logMsg, L"Loading shader: %s", path);
	LOG_INFO(L"Pixel Shader Loader", logMsg);

	HRESULT hr;
	
	ID3DBlob* pShaderBlob = NULL;
	hr = CompileShaderFromFile(path, options->EntryPoint, "ps_5_0", options->Defines, threadPump,
		errorMsg, errorLen, &pShaderBlob, NULL);
	if (FAILED(hr))
	{
		// CompileShaderFromFile sets the error message
		SAFE_RELEASE(pShaderBlob);
		return hr;
	}

	ID3D11PixelShader* ps;
	hr = device->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), NULL, &ps);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pShaderBlob);

		FormatDXErrorMessageW(hr, errorMsg, errorLen);
		return hr;
	}

	SAFE_RELEASE(pShaderBlob);

	if (options->DebugName)
	{
		SET_DEBUG_NAME(ps, options->DebugName);
	}

	PixelShaderContent* content = new PixelShaderContent();
	content->PixelShader = ps;

	*contentOut = content;
	return hr;
}