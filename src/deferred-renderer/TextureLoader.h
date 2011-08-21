#pragma once

#include "PCH.h"
#include "ContentType.h"
#include "ContentLoader.h"

struct TextureContent : public ContentType
{
	ID3D11ShaderResourceView* ShaderResourceView;
	D3DX11_IMAGE_INFO Info;

	TextureContent() : ShaderResourceView(NULL) { }
	~TextureContent() {  SAFE_RELEASE(ShaderResourceView); }
};

struct TextureLoadOptions
{
	bool Generate3DFrom2D;
	const char* DebugName;
};

template <>
HRESULT GenerateContentHash<TextureLoadOptions>(const WCHAR* path, TextureLoadOptions* options, ContentHash* hash);

class TextureLoader : public ContentLoader<TextureLoadOptions, TextureContent>
{
public:
	HRESULT Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		TextureLoadOptions* options, WCHAR* errorMsg, UINT errorLen, TextureContent** contentOut);
};