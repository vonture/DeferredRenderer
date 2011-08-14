#pragma once

#include "PCH.h"
#include "ContentTypeNoOptions.h"

class Texture2DContent : public ContentTypeNoOptions
{
private:
	ID3D11ShaderResourceView* _srv;
	D3DX11_IMAGE_INFO _info;

public:
	Texture2DContent(const WCHAR* path) : ContentTypeNoOptions(path) { }

	ID3D11ShaderResourceView* GetSRV() { return _srv; }
	const D3DX11_IMAGE_INFO* GetInfo() const { return &_info; }

	HRESULT LoadContent(ID3D11Device* device, const WCHAR* path);
	void UnloadContent();
};