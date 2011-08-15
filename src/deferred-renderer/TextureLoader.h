#pragma once

#include "PCH.h"
#include "TextureContent.h"
#include "ContentLoader.h"

struct TextureOptions
{
	const char* DebugName;
};

template <>
HRESULT GenerateContentHash<TextureOptions>(const WCHAR* path, TextureOptions* options, long* hash);

class Texture2DLoader : public ContentLoader<TextureOptions, Texture2DContent>
{
public:
	HRESULT Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		TextureOptions* options, WCHAR* errorMsg, UINT errorLen, Texture2DContent** contentOut);
};