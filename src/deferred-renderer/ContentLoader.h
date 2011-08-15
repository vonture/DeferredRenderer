#pragma once

#include "PCH.h"

template <class T>
HRESULT GenerateContentHash(const WCHAR* path, T* options, long* hash)
{
	return E_FAIL;
}

class ContentLoaderBase
{
private:
	virtual void MakeMePolymorphic() { };
};

template <class optionsType, class contentType>
class ContentLoader : public ContentLoaderBase
{
public:
	virtual HRESULT Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		optionsType* options, WCHAR* errorMsg, UINT errorLen, contentType** contentOut) = 0;
};