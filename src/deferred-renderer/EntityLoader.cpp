#include "PCH.h"
#include "EntityLoader.h"
#include "tinyxml.h"

HRESULT EntityLoader::GenerateContentHash(const WCHAR* path, EntityOptions* options, ContentHash* hash)
{
	if (!hash)
	{
		return FWP_E_NULL_POINTER;
	}

	locale loc;
	const collate<WCHAR>& wcoll = use_facet<collate<WCHAR>>(loc);

	*hash = wcoll.hash(path, path + wcslen(path));
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