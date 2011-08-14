#pragma once

#include "PCH.h"

class ContentTypeBase
{
public:	
	virtual HRESULT LoadContent(ID3D11Device* device) = 0;
	virtual void UnloadContent() = 0;
};

template <class T>
class ContentType : public ContentTypeBase
{
	const WCHAR* _path;
	T* _options;

public:
	ContentType(const WCHAR* path, T* options) 
		: _path(path), _options(options)
	{
	}
	
	virtual HRESULT LoadContent(ID3D11Device* device, const WCHAR* path, T* options) = 0;

	HRESULT LoadContent(ID3D11Device* device) 
	{
		return LoadContent(device, _path, _options); 
	}
};