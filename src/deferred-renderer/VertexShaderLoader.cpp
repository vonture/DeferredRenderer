#include "PCH.h"
#include "VertexShaderLoader.h"
#include "Logger.h"

HRESULT VertexShaderLoader::GenerateContentHash(const WCHAR* path, VertexShaderOptions* options, ContentHash* hash)
{
	if (!hash)
	{
		return FWP_E_NULL_POINTER;
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

		for (UINT i = 0; i < options->InputElementCount; i++)
		{
			AnsiToWString(options->InputElements[i].SemanticName, wBuff, 1024);
			retHash.append(wBuff);
		}

		// Not hashing the debug name since it does not affect the content that is loaded
	}

	*hash = retHash;
	return S_OK;
}


//
//	Vertex Shader Load
//
HRESULT VertexShaderLoader::LoadFromContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
	VertexShaderOptions* options, WCHAR* errorMsg, UINT errorLen, VertexShaderContent** contentOut)
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
	LOG_INFO(L"Vertex Shader Loader", logMsg);

	HRESULT hr;
	
	ID3DBlob* pShaderBlob = NULL;
	hr = CompileShaderFromFile(path, options->EntryPoint, "vs_5_0", options->Defines, threadPump,
		errorMsg, errorLen, &pShaderBlob, NULL);
	if (FAILED(hr))
	{
		// CompileShaderFromFile sets the error message
		SAFE_RELEASE(pShaderBlob);
		return hr;
	}

	ID3D11VertexShader* vs;
	hr = device->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), NULL, &vs);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pShaderBlob);

		FormatDXErrorMessageW(hr, errorMsg, errorLen);
		return hr;
	}

	ID3D11InputLayout* layout = NULL;
	hr = device->CreateInputLayout(options->InputElements, options->InputElementCount,
		pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), &layout);
	if (FAILED(hr))
	{
		SAFE_RELEASE(pShaderBlob);
		SAFE_RELEASE(vs);

		FormatDXErrorMessageW(hr, errorMsg, errorLen);
		return hr;
	}

	SAFE_RELEASE(pShaderBlob);

	if (options->DebugName)
	{
		CHAR debugName[256];

		sprintf_s(debugName, "%s %s", options->DebugName, "VS");
		SET_DEBUG_NAME(vs, debugName);

		sprintf_s(debugName, "%s %s", options->DebugName, "IL");
		SET_DEBUG_NAME(layout, debugName);
	}

	VertexShaderContent* content = new VertexShaderContent();
	content->VertexShader = vs;
	content->InputLayout = layout;

	*contentOut = content;
	return S_OK;
}

HRESULT VertexShaderLoader::LoadFromCompiledContentFile(ID3D11Device* device, const WCHAR* path, WCHAR* errorMsg,
		UINT errorLen, VertexShaderContent** contentOut)
{
	return E_NOTIMPL;
}