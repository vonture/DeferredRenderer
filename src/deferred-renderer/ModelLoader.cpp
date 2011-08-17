#include "PCH.h"
#include "ModelLoader.h"
#include "Logger.h"

template <>
HRESULT GenerateContentHash<ModelOptions>(const WCHAR* path, ModelOptions* options, long* hash)
{
	if (!hash)
	{
		return E_FAIL;
	}

	locale loc;
	const collate<WCHAR>& wcoll = use_facet<collate<WCHAR>>(loc);

	*hash = wcoll.hash(path, path + wcslen(path));
	return S_OK;
}

HRESULT ModelLoader::Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		ModelOptions* options, WCHAR* errorMsg, UINT errorLen, ModelContent** contentOut)
{
	WCHAR logMsg[MAX_LOG_LENGTH];
	swprintf_s(logMsg, L"Loading - %s", path);
	LOG_INFO(L"Model Loader", logMsg);

	Model* model = new Model();

	HRESULT hr = model->CreateFromFile(device, path);
	if (FAILED(hr))
	{
		delete model;
		return hr;
	}

	*contentOut = new ModelContent();
	(*contentOut)->Model = model;

	return S_OK;
}