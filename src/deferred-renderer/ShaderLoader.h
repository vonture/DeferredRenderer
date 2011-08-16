#pragma once

#include "PCH.h"

// TO BE REMOVED
HRESULT CompileShaderFromFile(WCHAR* szFileName,  LPCSTR szEntryPoint, LPCSTR szShaderModel, 
	D3D_SHADER_MACRO* defines, ID3DBlob** ppBlobOut);