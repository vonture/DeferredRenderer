#pragma once

#include "PCH.h"
#include "ContentLoader.h"
#include "ContentType.h"
#include "Model.h"

struct ModelContent : public ContentType
{
	Model* Model;

	ModelContent() : Model(NULL) { }
	~ModelContent() { SAFE_RELEASE(Model); }
};

struct ModelOptions
{
};

template <>
HRESULT GenerateContentHash<ModelOptions>(const WCHAR* path, ModelOptions* options, long* hash);

class ModelLoader : public ContentLoader<ModelOptions, ModelContent>
{
public:
	HRESULT Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		ModelOptions* options, WCHAR* errorMsg, UINT errorLen, ModelContent** contentOut);
};