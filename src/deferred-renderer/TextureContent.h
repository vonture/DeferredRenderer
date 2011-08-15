#pragma once

#include "PCH.h"
#include "ContentType.h"

struct Texture2DContent : public ContentType
{
	ID3D11ShaderResourceView* ShaderResourceView;
	D3DX11_IMAGE_INFO Info;

	Texture2DContent() : ShaderResourceView(NULL) { }
	~Texture2DContent() { SAFE_RELEASE(ShaderResourceView); }
};