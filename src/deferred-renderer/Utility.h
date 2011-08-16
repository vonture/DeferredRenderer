#pragma once

const float Pi = 3.14159f;
const float PiOver2 = Pi / 2.0f;
const float PiOver4 = Pi / 4.0f;
const float PiOver8 = Pi / 8.0f;

#define EPSILON 0.00001f

#ifndef clamp
#define clamp(val, min, max) (((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)))
#endif

#ifndef saturate
#define saturate(val) clamp((val), 0, 1)
#endif

inline HRESULT FormatHRESULTErrorMessageW(HRESULT errorId, WCHAR* msgBuffer, UINT msgLen)
{
	return FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorId, 
		LANG_SYSTEM_DEFAULT,
		msgBuffer,
		msgLen,
		NULL
	);
}

inline HRESULT FormatHRESULTErrorMessageA(HRESULT errorId, char* msgBuffer, UINT msgLen)
{
	return FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorId, 
		LANG_SYSTEM_DEFAULT,
		msgBuffer,
		msgLen,
		NULL
	);
}

inline HRESULT FormatDXErrorMessageW(HRESULT error, WCHAR* msgBuffer, UINT msgLen)
{
	return wcsncpy_s(msgBuffer, msgLen, DXGetErrorDescription(error), msgLen);
}

inline int AnsiToWString(const char* ansiString, WCHAR* output, UINT outputLength)
{
    return MultiByteToWideChar(CP_ACP, 0, ansiString, -1, output, outputLength);
}

inline int WStringToAnsi(const WCHAR* wString, char* output, UINT outputLength)
{
	return WideCharToMultiByte(CP_ACP, 0, wString, -1, output, outputLength, NULL, false);
}

inline int GetExtensionFromFileNameW(const WCHAR* fileName, WCHAR* output, UINT outputLength)
{
	_ASSERT(fileName);

	std::wstring str = std::wstring(fileName);
	UINT dotIdx = str.find_last_of(L".");

	if (dotIdx != str.npos)
	{
		wcsncpy_s(output, outputLength, str.substr(dotIdx).c_str(), outputLength);
		return 1;
	}
	else
	{
		return 0;
	}
}

inline int GetDirectoryFromFileNameW(const WCHAR* fileName, WCHAR* output, UINT outputLength)
{
    _ASSERT(fileName);

    std::wstring filePath(fileName);
    size_t idx = filePath.rfind(L'\\');
    if (idx != std::wstring::npos)
	{
		wcsncpy_s(output, outputLength, filePath.substr(0, idx + 1).c_str(), outputLength);
	}
    else
	{
        wcsncpy_s(output, outputLength, L"", outputLength);
	}

	return 1;
}

inline int GetDirectoryFromFileNameS(const char* fileName, char* output, UINT outputLength)
{
    _ASSERT(fileName);

    std::string filePath(fileName);
    size_t idx = filePath.rfind(L'\\');
    if (idx != std::wstring::npos)
	{
		strcpy_s(output, outputLength, filePath.substr(0, idx + 1).c_str());
	}
    else
	{
        strcpy_s(output, outputLength, "");
	}

	return 1;
}

inline HRESULT CompileShaderFromFile(const WCHAR* szFileName, const char* szEntryPoint, const char* szShaderModel,
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
        if(pErrorBlob)
		{
			AnsiToWString((char*)pErrorBlob->GetBufferPointer(), errorBuffer, errorLen);
			pErrorBlob->Release();
		}
        
        return hr;
    }

	if (pErrorBlob)
	{
		pErrorBlob->Release();
	}

    return S_OK;
}

#define SET_DEBUG_NAME(obj, name) ((obj)->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), (name)))

#if defined(DEBUG) || defined(_DEBUG)
	#ifndef V
		#define V(x)           { hr = (x); if( FAILED(hr) ) { DXTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); } }
	#endif
	#ifndef V_WIN
		#define V_WIN(x) { if (!(x)) { hr = HRESULT_FROM_WIN32(GetLastError()); DXTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true); } }
	#endif

	#ifndef V_RETURN
		#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return DXTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true ); } }
	#endif
	#ifndef V_WIN_RETURN
		#define V_WIN_RETURN(x) { if (!(x)) { hr = HRESULT_FROM_WIN32(GetLastError()); return DXTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true); } else { hr = S_OK; } }
	#endif
#else
	#ifndef V
		#define V(x)           { hr = (x); }
	#endif
	#ifndef V_WIN
		#define V_WIN(x)       { if (!(x)) { hr = HRESULT_FROM_WIN32(GetLastError()); } else { hr = S_OK; } }
	#endif

	#ifndef V_RETURN
		#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return hr; } }
	#endif
	#ifndef V_WIN_RETURN
		#define V_WIN_RETURN(x) { if (!(x)) { hr = HRESULT_FROM_WIN32(GetLastError()); } else { hr = S_OK; } }
	#endif
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif