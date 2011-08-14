#pragma once

#include "PCH.h"
#include "ContentType.h"

struct EmptyOptions
{
};

class ContentTypeNoOptions : public ContentType<EmptyOptions>
{
public:
	ContentTypeNoOptions(const WCHAR* path) 
		: ContentType(path, NULL)
	{
	}

	virtual HRESULT LoadContent(ID3D11Device* device, const WCHAR* path) = 0;

	HRESULT LoadContent(ID3D11Device* device, const WCHAR* path, EmptyOptions* options)
	{
		return LoadContent(device, path);
	}
};