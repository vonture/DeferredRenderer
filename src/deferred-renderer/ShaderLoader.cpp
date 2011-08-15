#include "PCH.h"
#include "ShaderLoader.h"

// TO BE REMOVED
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel,
	D3D_SHADER_MACRO* defines, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    // find the file
    WCHAR str[MAX_PATH];
    V_RETURN(DXUTFindDXSDKMediaFileCch(str, MAX_PATH, szFileName));

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile(str, defines, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        SAFE_RELEASE( pErrorBlob );
        return hr;
    }
    SAFE_RELEASE( pErrorBlob );

    return S_OK;
}

template <>
HRESULT GenerateContentHash<ShaderOptions>(const WCHAR* path, ShaderOptions* options, long* hash)
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

HRESULT CompileShaderFromFile(const WCHAR* szFileName, const char* szEntryPoint, const char* szShaderModel,
	D3D_SHADER_MACRO* defines, ID3DX11ThreadPump* threadPump, WCHAR* errorBuffer, UINT errorLen, 
	ID3DBlob** ppBlobOut, HRESULT* hrOut)
{
    HRESULT hr;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile(szFileName, defines, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
    if(FAILED(hr))
    {
        if(pErrorBlob != NULL)
		{
			AnsiToWString((char*)pErrorBlob->GetBufferPointer(), errorBuffer, errorLen);
		}
        SAFE_RELEASE(pErrorBlob);
        return hr;
    }
    SAFE_RELEASE(pErrorBlob);

    return S_OK;
}

HRESULT PixelShaderLoader::Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
	ShaderOptions* options, WCHAR* errorMsg, UINT errorLen, PixelShaderContent** contentOut)
{
	if (!options)
	{
		return E_FAIL;
	}

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

		FormatHRESULTErrorMessageW(hr, errorMsg, errorLen);
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
