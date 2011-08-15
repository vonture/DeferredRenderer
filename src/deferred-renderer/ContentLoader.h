#pragma once

#include "PCH.h"

class ContentLoaderBase
{
};

template <class T>
HRESULT GenerateContentHash(const WCHAR* path, T* options, long* hash)
{
	return E_FAIL;
}

template <class optionsType, class contentType>
class ContentLoader
{
public:
	HRESULT Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		optionsType* options, WCHAR* errorMsg, UINT errorLen, contentType** contentOut);
};