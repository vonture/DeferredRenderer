//--------------------------------------------------------------------------------------
// File: SDKmisc.cpp
//
// Various helper functionality that is shared between SDK samples
//
// Copyright (c) Microsoft Corporation. All rights reserved
//--------------------------------------------------------------------------------------
#include "PCH.h"
#include "SDKmisc.h"
#undef min // use __min instead
#undef max // use __max instead

//--------------------------------------------------------------------------------------
// Internal functions forward declarations
//--------------------------------------------------------------------------------------
bool DXUTFindMediaSearchTypicalDirs( __out_ecount(cchSearch) WCHAR* strSearchPath, 
                                     __in int cchSearch, 
                                     __in LPCWSTR strLeaf, 
                                     __in WCHAR* strExePath,
                                     __in WCHAR* strExeName );
bool DXUTFindMediaSearchParentDirs( __out_ecount(cchSearch) WCHAR* strSearchPath, 
                                    __in int cchSearch, 
                                    __in WCHAR* strStartAt, 
                                    __in WCHAR* strLeafName );

//--------------------------------------------------------------------------------------
// Returns pointer to static media search buffer
//--------------------------------------------------------------------------------------
WCHAR* DXUTMediaSearchPath()
{
    static WCHAR s_strMediaSearchPath[MAX_PATH] =
    {
        0
    };
    return s_strMediaSearchPath;

}

//--------------------------------------------------------------------------------------
LPCWSTR WINAPI DXUTGetMediaSearchPath()
{
    return DXUTMediaSearchPath();
}


//--------------------------------------------------------------------------------------
HRESULT WINAPI DXUTSetMediaSearchPath( LPCWSTR strPath )
{
    HRESULT hr;

    WCHAR* s_strSearchPath = DXUTMediaSearchPath();

    hr = wcscpy_s( s_strSearchPath, MAX_PATH, strPath );
    if( SUCCEEDED( hr ) )
    {
        // append slash if needed
        size_t ch = 0;
        ch = wcsnlen( s_strSearchPath, MAX_PATH);
        if( SUCCEEDED( hr ) && s_strSearchPath[ch - 1] != L'\\' )
        {
            hr = wcscat_s( s_strSearchPath, MAX_PATH, L"\\" );
        }
    }

    return hr;
}


//--------------------------------------------------------------------------------------
// Tries to find the location of a SDK media file
//       cchDest is the size in WCHARs of strDestPath.  Be careful not to 
//       pass in sizeof(strDest) on UNICODE builds.
//--------------------------------------------------------------------------------------
HRESULT WINAPI DXUTFindDXSDKMediaFileCch( WCHAR* strDestPath, int cchDest, 
                                          LPCWSTR strFilename )
{
    bool bFound;
    WCHAR strSearchFor[MAX_PATH];

    if( NULL == strFilename || strFilename[0] == 0 || NULL == strDestPath || cchDest < 10 )
        return E_INVALIDARG;

    // Get the exe name, and exe path
    WCHAR strExePath[MAX_PATH] =
    {
        0
    };
    WCHAR strExeName[MAX_PATH] =
    {
        0
    };
    WCHAR* strLastSlash = NULL;
    GetModuleFileName( NULL, strExePath, MAX_PATH );
    strExePath[MAX_PATH - 1] = 0;
    strLastSlash = wcsrchr( strExePath, TEXT( '\\' ) );
    if( strLastSlash )
    {
        wcscpy_s( strExeName, MAX_PATH, &strLastSlash[1] );

        // Chop the exe name from the exe path
        *strLastSlash = 0;

        // Chop the .exe from the exe name
        strLastSlash = wcsrchr( strExeName, TEXT( '.' ) );
        if( strLastSlash )
            *strLastSlash = 0;
    }

    // Typical directories:
    //      .\
    //      ..\
    //      ..\..\
    //      %EXE_DIR%\
    //      %EXE_DIR%\..\
    //      %EXE_DIR%\..\..\
    //      %EXE_DIR%\..\%EXE_NAME%
    //      %EXE_DIR%\..\..\%EXE_NAME%

    // Typical directory search
    bFound = DXUTFindMediaSearchTypicalDirs( strDestPath, cchDest, strFilename, strExePath, strExeName );
    if( bFound )
        return S_OK;

    // Typical directory search again, but also look in a subdir called "\media\" 
    swprintf_s( strSearchFor, MAX_PATH, L"media\\%s", strFilename );
    bFound = DXUTFindMediaSearchTypicalDirs( strDestPath, cchDest, strSearchFor, strExePath, strExeName );
    if( bFound )
        return S_OK;

    WCHAR strLeafName[MAX_PATH] =
    {
        0
    };

    // Search all parent directories starting at .\ and using strFilename as the leaf name
    wcscpy_s( strLeafName, MAX_PATH, strFilename );
    bFound = DXUTFindMediaSearchParentDirs( strDestPath, cchDest, L".", strLeafName );
    if( bFound )
        return S_OK;

    // Search all parent directories starting at the exe's dir and using strFilename as the leaf name
    bFound = DXUTFindMediaSearchParentDirs( strDestPath, cchDest, strExePath, strLeafName );
    if( bFound )
        return S_OK;

    // Search all parent directories starting at .\ and using "media\strFilename" as the leaf name
    swprintf_s( strLeafName, MAX_PATH, L"media\\%s", strFilename );
    bFound = DXUTFindMediaSearchParentDirs( strDestPath, cchDest, L".", strLeafName );
    if( bFound )
        return S_OK;

    // Search all parent directories starting at the exe's dir and using "media\strFilename" as the leaf name
    bFound = DXUTFindMediaSearchParentDirs( strDestPath, cchDest, strExePath, strLeafName );
    if( bFound )
        return S_OK;

    // On failure, return the file as the path but also return an error code
    wcscpy_s( strDestPath, cchDest, strFilename );

    return E_FAIL;
}


//--------------------------------------------------------------------------------------
// Search a set of typical directories
//--------------------------------------------------------------------------------------
bool DXUTFindMediaSearchTypicalDirs( WCHAR* strSearchPath, int cchSearch, LPCWSTR strLeaf, 
                                     WCHAR* strExePath, WCHAR* strExeName )
{
    // Typical directories:
    //      .\
    //      ..\
    //      ..\..\
    //      %EXE_DIR%\
    //      %EXE_DIR%\..\
    //      %EXE_DIR%\..\..\
    //      %EXE_DIR%\..\%EXE_NAME%
    //      %EXE_DIR%\..\..\%EXE_NAME%
    //      DXSDK media path

    // Search in .\  
    wcscpy_s( strSearchPath, cchSearch, strLeaf );
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    // Search in ..\  
    swprintf_s( strSearchPath, cchSearch, L"..\\%s", strLeaf );
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    // Search in ..\..\ 
    swprintf_s( strSearchPath, cchSearch, L"..\\..\\%s", strLeaf );
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    // Search in ..\..\ 
    swprintf_s( strSearchPath, cchSearch, L"..\\..\\%s", strLeaf );
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    // Search in the %EXE_DIR%\ 
    swprintf_s( strSearchPath, cchSearch, L"%s\\%s", strExePath, strLeaf );
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    // Search in the %EXE_DIR%\..\ 
    swprintf_s( strSearchPath, cchSearch, L"%s\\..\\%s", strExePath, strLeaf );
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    // Search in the %EXE_DIR%\..\..\ 
    swprintf_s( strSearchPath, cchSearch, L"%s\\..\\..\\%s", strExePath, strLeaf );
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    // Search in "%EXE_DIR%\..\%EXE_NAME%\".  This matches the DirectX SDK layout
    swprintf_s( strSearchPath, cchSearch, L"%s\\..\\%s\\%s", strExePath, strExeName, strLeaf );
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    // Search in "%EXE_DIR%\..\..\%EXE_NAME%\".  This matches the DirectX SDK layout
    swprintf_s( strSearchPath, cchSearch, L"%s\\..\\..\\%s\\%s", strExePath, strExeName, strLeaf );
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    // Search in media search dir 
    WCHAR* s_strSearchPath = DXUTMediaSearchPath();
    if( s_strSearchPath[0] != 0 )
    {
        swprintf_s( strSearchPath, cchSearch, L"%s%s", s_strSearchPath, strLeaf );
        if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
            return true;
    }

    return false;
}



//--------------------------------------------------------------------------------------
// Search parent directories starting at strStartAt, and appending strLeafName
// at each parent directory.  It stops at the root directory.
//--------------------------------------------------------------------------------------
bool DXUTFindMediaSearchParentDirs( WCHAR* strSearchPath, int cchSearch, WCHAR* strStartAt, 
                                    WCHAR* strLeafName )
{
    WCHAR strFullPath[MAX_PATH] =
    {
        0
    };
    WCHAR strFullFileName[MAX_PATH] =
    {
        0
    };
    WCHAR strSearch[MAX_PATH] =
    {
        0
    };
    WCHAR* strFilePart = NULL;

    GetFullPathName( strStartAt, MAX_PATH, strFullPath, &strFilePart );
    if( strFilePart == NULL )
        return false;

    while( strFilePart != NULL && *strFilePart != '\0' )
    {
        swprintf_s( strFullFileName, MAX_PATH, L"%s\\%s", strFullPath, strLeafName );
        if( GetFileAttributes( strFullFileName ) != 0xFFFFFFFF )
        {
            wcscpy_s( strSearchPath, cchSearch, strFullFileName );
            return true;
        }

        swprintf_s( strSearch, MAX_PATH, L"%s\\..", strFullPath );
        GetFullPathName( strSearch, MAX_PATH, strFullPath, &strFilePart );
    }

    return false;
}

//--------------------------------------------------------------------------------------
// Helper functions to create SRGB formats from typeless formats and vice versa
//--------------------------------------------------------------------------------------
DXGI_FORMAT MAKE_SRGB( DXGI_FORMAT format )
{
    switch( format )
    {
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
            return DXGI_FORMAT_BC1_UNORM_SRGB;
        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
            return DXGI_FORMAT_BC2_UNORM_SRGB;
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
            return DXGI_FORMAT_BC3_UNORM_SRGB;

    };

    return format;
}

//--------------------------------------------------------------------------------------
DXGI_FORMAT MAKE_TYPELESS( DXGI_FORMAT format )
{
    switch( format )
    {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
            return DXGI_FORMAT_R8G8B8A8_TYPELESS;

        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC1_UNORM:
            return DXGI_FORMAT_BC1_TYPELESS;
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC2_UNORM:
            return DXGI_FORMAT_BC2_TYPELESS;
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC3_UNORM:
            return DXGI_FORMAT_BC3_TYPELESS;
    };

    return format;
}