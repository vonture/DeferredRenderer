#pragma once

#include "PCH.h"
#include "ContentLoader.h"
#include "ContentType.h"

struct VertexShaderContent : public ContentType
{
	ID3D11VertexShader* VertexShader;
	ID3D11InputLayout* InputLayout;

	VertexShaderContent() : VertexShader(NULL) { }
	~VertexShaderContent() { SAFE_RELEASE(InputLayout); SAFE_RELEASE(VertexShader); }
};

struct VertexShaderOptions
{
	const char* EntryPoint;
	D3D_SHADER_MACRO* Defines;
	D3D11_INPUT_ELEMENT_DESC* InputElements;
	UINT InputElementCount;
	const char* DebugName;
};

template <>
HRESULT GenerateContentHash<VertexShaderOptions>(const WCHAR* path, VertexShaderOptions* options, long* hash);

class VertexShaderLoader : public ContentLoader<VertexShaderOptions, VertexShaderContent>
{
public:
	HRESULT Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		VertexShaderOptions* options, WCHAR* errorMsg, UINT errorLen, VertexShaderContent** contentOut);
};