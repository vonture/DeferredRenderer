#pragma once

#include "DXUT.h"
#include "SDKmisc.h"

HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );