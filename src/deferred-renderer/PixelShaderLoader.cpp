#include "PCH.h"
#include "PixelShaderLoader.h"
#include "Logger.h"

HRESULT PixelShaderLoader::GenerateContentHash(const WCHAR* path, PixelShaderOptions* options, ContentHash* hash)
{
	if (!hash)
	{
		return E_FAIL;
	}
	
	ContentHash retHash;
	retHash.append(path);

	if (options)
	{
		WCHAR wBuff[1024];
		AnsiToWString(options->EntryPoint, wBuff, 1024);
		retHash.append(wBuff);

		if (options->Defines)
		{
			UINT defineIdx = 0;
			while (options->Defines[defineIdx].Name)
			{
				AnsiToWString(options->Defines[defineIdx].Name, wBuff, 1024);
				retHash.append(wBuff);

				AnsiToWString(options->Defines[defineIdx].Definition, wBuff, 1024);
				retHash.append(wBuff);

				defineIdx++;
			}
		}
		
		// Not hashing the debug name since it does not affect the content that is loaded
	}

	*hash = retHash;
	return S_OK;
}

HRESULT PixelShaderLoader::LoadFromContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
	PixelShaderOptions* options, WCHAR* errorMsg, UINT errorLen, PixelShaderContent** contentOut)
{
	if (!options)
	{
		swprintf_s(errorMsg, errorLen, L"Options cannot be null when loading shaders.");
		return E_FAIL;
	}
	
	WCHAR logMsg[MAX_LOG_LENGTH];
	if (options->DebugName)
	{
		WCHAR debugNameW[256];
		AnsiToWString(options->DebugName, debugNameW, 256);
		
		swprintf_s(logMsg, L"Loading - %s (path = %s)", debugNameW, path);		
	}
	else
	{
		swprintf_s(logMsg, L"Loading - %s", path);
	}
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
		CHAR debugName[256];

		sprintf_s(debugName, "%s %s", options->DebugName, "PS");
		V_RETURN(SetDXDebugName(ps, debugName));
	}

	PixelShaderContent* content = new PixelShaderContent();
	content->PixelShader = ps;

	*contentOut = content;
	return S_OK;
}

HRESULT PixelShaderLoader::LoadFromCompiledContentFile(ID3D11Device* device, const WCHAR* path, WCHAR* errorMsg,
		UINT errorLen, PixelShaderContent** contentOut)
{
	return E_NOTIMPL;
}