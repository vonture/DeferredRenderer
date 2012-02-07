#pragma once

#include "PCH.h"
#include "ContentLoader.h"
#include "ContentType.h"

struct VertexShaderContent : public ContentType
{
	ID3D11VertexShader* VertexShader;
	ID3D11InputLayout* InputLayout;

	VertexShaderContent() : VertexShader(NULL), InputLayout(NULL) { }
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

class VertexShaderLoader : public ContentLoader<VertexShaderOptions, VertexShaderContent>
{
public:
	HRESULT GenerateContentHash(const WCHAR* path, VertexShaderOptions* options, ContentHash* hash);
	HRESULT CompileContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump, 
		const WCHAR* path, VertexShaderOptions* options, WCHAR* errorMsg, UINT errorLen, std::ostream* output);
	HRESULT LoadFromCompiledContentFile(ID3D11Device* device, std::istream* input, VertexShaderOptions* options, 
		WCHAR* errorMsg, UINT errorLen, VertexShaderContent** contentOut);
};