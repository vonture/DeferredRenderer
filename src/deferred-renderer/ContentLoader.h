#pragma once

#include "PCH.h"

typedef std::wstring ContentHash;

class ContentLoaderBase
{
private:
	virtual void MakeMePolymorphic() { };
};

template <class optionsType, class contentType>
class ContentLoader : public ContentLoaderBase
{
public:
	virtual HRESULT GenerateContentHash(const WCHAR* path, optionsType* options, ContentHash* hash) = 0;

	virtual HRESULT LoadFromContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump, 
		const WCHAR* path, optionsType* options, WCHAR* errorMsg, UINT errorLen, contentType** contentOut) = 0;

	virtual HRESULT LoadFromCompiledContentFile(ID3D11Device* device, const WCHAR* path, WCHAR* errorMsg,
		UINT errorLen, contentType** contentOut) = 0;
};