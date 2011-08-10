//--------------------------------------------------------------------------------------
// File: SDKMisc.h
//
// Various helper functionality that is shared between SDK samples
//
// Copyright (c) Microsoft Corporation. All rights reserved
//--------------------------------------------------------------------------------------
#pragma once
#ifndef SDKMISC_H
#define SDKMISC_H

#include "PCH.h"

//--------------------------------------------------------------------------------------
// Tries to finds a media file by searching in common locations
//--------------------------------------------------------------------------------------
HRESULT WINAPI DXUTFindDXSDKMediaFileCch( __out_ecount(cchDest) WCHAR* strDestPath,
                                          __in int cchDest, 
                                          __in LPCWSTR strFilename );
HRESULT WINAPI DXUTSetMediaSearchPath( LPCWSTR strPath );
LPCWSTR WINAPI DXUTGetMediaSearchPath();

//--------------------------------------------------------------------------------------
// Helper functions to create SRGB formats from typeless formats and vice versa
//--------------------------------------------------------------------------------------
DXGI_FORMAT MAKE_SRGB( DXGI_FORMAT format );
DXGI_FORMAT MAKE_TYPELESS( DXGI_FORMAT format );

#endif
