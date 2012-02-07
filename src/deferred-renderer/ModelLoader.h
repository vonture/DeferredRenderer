#pragma once

#include "PCH.h"
#include "ContentLoader.h"
#include "ContentType.h"
#include "Model.h"

struct ModelOptions
{
};

class ModelLoader : public ContentLoader<ModelOptions, Model>
{
public:
	HRESULT GenerateContentHash(const WCHAR* path, ModelOptions* options, ContentHash* hash);
	HRESULT CompileContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump, 
		const WCHAR* path, ModelOptions* options, WCHAR* errorMsg, UINT errorLen, std::ostream* output);
	HRESULT LoadFromCompiledContentFile(ID3D11Device* device, std::istream* input, ModelOptions* options, 
		WCHAR* errorMsg, UINT errorLen, Model** contentOut);
};