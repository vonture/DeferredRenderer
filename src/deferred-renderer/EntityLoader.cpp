#include "PCH.h"
#include "EntityLoader.h"
#include "tinyxml.h"

HRESULT EntityLoader::GenerateContentHash(const WCHAR* path, EntityOptions* options, ContentHash* hash)
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

HRESULT EntityLoader::LoadFromContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		EntityOptions* options, WCHAR* errorMsg, UINT errorLen, EntityContent** contentOut)
{
	return E_NOTIMPL;
}

HRESULT EntityLoader::LoadFromCompiledContentFile(ID3D11Device* device, const WCHAR* path, WCHAR* errorMsg,
		UINT errorLen, EntityContent** contentOut)
{
	return E_NOTIMPL;
}