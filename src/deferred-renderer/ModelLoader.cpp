#include "PCH.h"
#include "ModelLoader.h"
#include "Logger.h"

HRESULT ModelLoader::GenerateContentHash(const WCHAR* path, ModelOptions* options, ContentHash* hash)
{
	if (!hash)
	{
		return FWP_E_NULL_POINTER;
	}

	ContentHash retHash;
	retHash.append(path);

	*hash = retHash;
	return S_OK;
}

HRESULT ModelLoader::LoadFromContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		ModelOptions* options, WCHAR* errorMsg, UINT errorLen, Model** contentOut)
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

	*contentOut = model;
	return S_OK;
}

HRESULT ModelLoader::LoadFromCompiledContentFile(ID3D11Device* device, const WCHAR* path, WCHAR* errorMsg,
		UINT errorLen, Model** contentOut)
{
	return E_NOTIMPL;
}