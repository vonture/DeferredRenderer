#pragma once

#include "PCH.h"
#include "ContentLoader.h"
#include "ContentType.h"
#include "Model.h"

struct ModelOptions
{
};

template <>
HRESULT GenerateContentHash<ModelOptions>(const WCHAR* path, ModelOptions* options, long* hash);

class ModelLoader : public ContentLoader<ModelOptions, Model>
{
public:
	HRESULT Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		ModelOptions* options, WCHAR* errorMsg, UINT errorLen, Model** contentOut);
};