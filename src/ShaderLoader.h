#pragma once

#include "Defines.h"
#include "SDKmisc.h"

HRESULT CompileShaderFromFile(WCHAR* szFileName,  LPCSTR szEntryPoint, LPCSTR szShaderModel, 
	D3D_SHADER_MACRO* defines, ID3DBlob** ppBlobOut);