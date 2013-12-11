#pragma once

const float Pi = 3.14159f;
const float PiOver2 = Pi / 2.0f;
const float PiOver4 = Pi / 4.0f;
const float PiOver8 = Pi / 8.0f;

#define EPSILON 0.00002f

#ifndef clamp
#define clamp(val, min, max) (((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)))
#endif

#ifndef saturate
#define saturate(val) clamp((val), 0, 1)
#endif

inline float RandomBetween(float min, float max)
{
    float perc = (float)(rand() % 10000) / 10000.0f;
    return min + ((max - min) * perc);
}

inline HRESULT WriteFileAndSizeToStream(const std::wstring& path, std::ostream& stream)
{
    UINT fileSize = 0;

    std::ifstream file;
    file.open(path, std::ios::in | std::ios::binary);
    if (file.is_open())
    {
        file.seekg(0, ios::end);
        fileSize = (UINT)file.tellg();
        file.seekg(0, ios::beg);
    }

    if (!stream.write((const char*)&fileSize, sizeof(UINT)))
    {
        return E_FAIL;
    }

    if (fileSize > 0)
    {
        BYTE* buf = new BYTE[fileSize];

        if (!file.read((char*)buf, fileSize))
        {
            delete[] buf;
            return E_FAIL;
        }

        if (!stream.write((const char*)buf, fileSize))
        {
            delete[] buf;
            return E_FAIL;
        }

        delete[] buf;
    }

    return S_OK;
}

inline HRESULT ReadFileFromStream(std::istream& stream, byte** buf, UINT& bufSize)
{
    if (!stream.read((char*)&bufSize, sizeof(UINT)))
    {
        return E_FAIL;
    }

    if (bufSize > 0)
    {
        *buf = new BYTE[bufSize];
        if (!stream.read((char*)*buf, bufSize))
        {
            delete[] (*buf);
            return E_FAIL;
        }
    }

    return S_OK;
}

inline std::wstring ReadWStringFromStream(std::istream& stream)
{
    UINT len;
    if (!stream.read((char*)&len, sizeof(UINT)))
    {
        return L"";
    }

    WCHAR* buf = new WCHAR[len + 1];
    buf[len] = L'\0';

    if (!stream.read((char*)buf, len * sizeof(WCHAR)))
    {
        delete[] buf;
        return L"";
    }

    std::wstring output = std::wstring(buf);
    delete[] buf;

    return output;
}

inline UINT WriteWStringToStream(const std::wstring& str, std::ostream& stream)
{
    UINT len = str.size();
    if (!stream.write((const char*)&len, sizeof(UINT)))
    {
        return 0;
    }

    if (!stream.write((const char*)str.c_str(), len * sizeof(WCHAR)))
    {
        return sizeof(UINT);
    }

    return sizeof(UINT) + len * sizeof(WCHAR);
}

template <typename T>
inline bool ReadDataFromStream(T& data, std::istream& stream)
{
    return !stream.read((char*)&data, sizeof(T)).bad();
}

template <typename T>
inline bool ReadDataArrayFromStream(T dataArray[], UINT count, std::istream& stream)
{
    return !stream.read((char*)dataArray, sizeof(T) * count).bad();
}

template <typename T>
inline bool WriteDataTostream(const T data, std::ostream& stream)
{
    return !stream.write((const char*)&data, sizeof(T)).bad();
}

template <typename T>
inline bool WriteDataArrayTostream(T dataArray[], UINT count, std::ostream& stream)
{
    return !stream.write((const char*)dataArray, sizeof(T) * count).bad();
}

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
    return WideCharToMultiByte(CP_ACP, 0, wString, -1, output, outputLength, NULL, NULL);
}

inline std::wstring AnsiToWString(const std::string& input)
{
    WCHAR* buf = new WCHAR[input.size() + 1];
    AnsiToWString(input.c_str(), buf, input.size() + 1);

    std::wstring output = std::wstring(buf);
    delete[] buf;

    return output;
}

inline std::string WStringToAnsi(const std::wstring& input)
{
    char* buf = new char[input.size() + 1];
    WStringToAnsi(input.c_str(), buf, input.size() + 1);

    std::string output = std::string(buf);
    delete[] buf;

    return output;
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

inline std::wstring GetExtensionFromFileNameW(const std::wstring& fileName)
{
    UINT dotIdx = fileName.find_last_of(L".");

    if (dotIdx != fileName.npos)
    {
        return fileName.substr(dotIdx);
    }
    else
    {
        return L"";
    }
}

inline int GetFileNameWithoutExtensionW(const WCHAR* fileName, WCHAR* output, UINT outputLength)
{
    _ASSERT(fileName);

    std::wstring str = std::wstring(fileName);

    UINT dotIdx = str.find_last_of(L".");
    if (dotIdx == std::wstring::npos)
    {
        dotIdx = str.size();
    }

    size_t pathIdx = str.find_last_of(L'\\');
    if (pathIdx == std::wstring::npos)
    {
        pathIdx = 0;
    }
    else
    {
        pathIdx++;
    }

    wcsncpy_s(output, outputLength, str.substr(pathIdx, dotIdx - pathIdx).c_str(), outputLength);
    return 1;
}

inline std::wstring GetFileNameWithoutExtensionW(const std::wstring& fileName)
{
    UINT dotIdx = fileName.find_last_of(L".");
    if (dotIdx == std::wstring::npos)
    {
        dotIdx = fileName.size();
    }

    size_t pathIdx = fileName.find_last_of(L'\\');
    if (pathIdx == std::wstring::npos)
    {
        pathIdx = 0;
    }
    else
    {
        pathIdx++;
    }

    return fileName.substr(pathIdx, dotIdx - pathIdx);
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

inline std::wstring GetDirectoryFromFileNameW(const std::wstring& input)
{
    size_t idx = input.rfind(L'\\');
    if (idx != std::wstring::npos)
    {
        return input.substr(0, idx + 1).c_str();
    }
    else
    {
        return L"";
    }
}

inline std::string GetDirectoryFromFileNameS(const std::string& input)
{
    size_t idx = input.rfind(L'\\');
    if (idx != std::wstring::npos)
    {
        return input.substr(0, idx + 1).c_str();
    }
    else
    {
        return "";
    }
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
    dwShaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_WARNINGS_ARE_ERRORS;
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

#if defined(PROFILE) || defined(DEBUG) || defined(_DEBUG)
inline HRESULT SetDXDebugName(IDirect3DResource9* pObj, const CHAR* pstrName)
{
    return pObj ? pObj->SetPrivateData(WKPDID_D3DDebugObjectName, pstrName, lstrlenA(pstrName), 0) : E_FAIL;
}

inline HRESULT SetDXDebugName(IDXGIObject* pObj, const CHAR* pstrName)
{
    return pObj ? pObj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(pstrName), pstrName) : E_FAIL;
}

inline HRESULT SetDXDebugName(ID3D11Device* pObj, const CHAR* pstrName)
{
    return pObj ? pObj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(pstrName), pstrName) : E_FAIL;
}

inline HRESULT SetDXDebugName(ID3D11DeviceChild* pObj, const CHAR* pstrName)
{
    return pObj ? pObj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(pstrName), pstrName) : E_FAIL;
}
#else
#define SetDXDebugName(pObj, pstrName) (S_OK)
#endif

#if defined(DEBUG) || defined(_DEBUG)
#ifndef V
#define V(x)           { hr = (x); if(FAILED(hr)) { DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); } }
#endif
#ifndef V_WIN
#define V_WIN(x) { if (!(x)) { hr = HRESULT_FROM_WIN32(GetLastError()); DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); } }
#endif

#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if(FAILED(hr)) { return DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); } }
#endif
#ifndef V_WIN_RETURN
#define V_WIN_RETURN(x) { if (!(x)) { hr = HRESULT_FROM_WIN32(GetLastError()); return DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); } else { hr = S_OK; } }
#endif
#else
#ifndef V
#define V(x)           { hr = (x); }
#endif
#ifndef V_WIN
#define V_WIN(x)       { if (!(x)) { hr = HRESULT_FROM_WIN32(GetLastError()); } else { hr = S_OK; } }
#endif

#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if(FAILED(hr)) { return hr; } }
#endif
#ifndef V_WIN_RETURN
#define V_WIN_RETURN(x) { if (!(x)) { hr = HRESULT_FROM_WIN32(GetLastError()); } else { hr = S_OK; } }
#endif
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p); (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p); (p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef SAFE_CM_RELEASE
#define SAFE_CM_RELEASE(cm, p) { if (p) { cm->ReleaseContent(p); (p)=NULL; } }
#endif