#pragma once

#include "PCH.h"
#include "ContentLoader.h"
#include "ContentType.h"

struct PixelShaderContent : public ContentType
{
	ID3D11PixelShader* PixelShader;

	PixelShaderContent() : PixelShader(NULL) { }
	~PixelShaderContent() { SAFE_RELEASE(PixelShader); }
};

struct PixelShaderOptions
{
	const char* EntryPoint;
	D3D_SHADER_MACRO* Defines;
	const char* DebugName;
};

class PixelShaderLoader : public ContentLoader<PixelShaderOptions, PixelShaderContent>
{
public:
	HRESULT GenerateContentHash(const WCHAR* path, PixelShaderOptions* options, ContentHash* hash);
	HRESULT LoadFromContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		PixelShaderOptions* options, WCHAR* errorMsg, UINT errorLen, PixelShaderContent** contentOut);
	HRESULT LoadFromCompiledContentFile(ID3D11Device* device, const WCHAR* path, WCHAR* errorMsg,
		UINT errorLen, PixelShaderContent** contentOut);
};