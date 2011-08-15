#pragma once

#include "PCH.h"
#include "ContentLoader.h"
#include "ShaderContent.h"

// TO BE REMOVED
HRESULT CompileShaderFromFile(WCHAR* szFileName,  LPCSTR szEntryPoint, LPCSTR szShaderModel, 
	D3D_SHADER_MACRO* defines, ID3DBlob** ppBlobOut);

struct ShaderOptions
{
	const char* EntryPoint;
	D3D_SHADER_MACRO* Defines;
	const char* DebugName;
};

template <>
HRESULT GenerateContentHash<ShaderOptions>(const WCHAR* path, ShaderOptions* options, long* hash);

class PixelShaderLoader : public ContentLoader<ShaderOptions, PixelShaderContent>
{
public:
	HRESULT Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		ShaderOptions* options, WCHAR* errorMsg, UINT errorLen, PixelShaderContent** contentOut);
};