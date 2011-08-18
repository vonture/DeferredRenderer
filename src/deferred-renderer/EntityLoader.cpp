#include "PCH.h"
#include "EntityLoader.h"
#include "tinyxml.h"

template <>
HRESULT GenerateContentHash<EntityOptions>(const WCHAR* path, EntityOptions* options, long* hash)
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

HRESULT EntityLoader::Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		EntityOptions* options, WCHAR* errorMsg, UINT errorLen, EntityContent** contentOut)
{
	return E_NOTIMPL;
}