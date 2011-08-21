#include "PCH.h"
#include "EntityLoader.h"
#include "tinyxml.h"

template <>
HRESULT GenerateContentHash<EntityOptions>(const WCHAR* path, EntityOptions* options, ContentHash* hash)
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

HRESULT EntityLoader::Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		EntityOptions* options, WCHAR* errorMsg, UINT errorLen, EntityContent** contentOut)
{
	return E_NOTIMPL;
}