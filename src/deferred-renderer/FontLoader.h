#pragma once

#include "PCH.h"
#include "ContentLoader.h"
#include "ContentType.h"
#include "SpriteFont.h"

struct FontOptions
{
};

template <>
HRESULT GenerateContentHash<FontOptions>(const WCHAR* path, FontOptions* options, long* hash);

class FontLoader : public ContentLoader<FontOptions, SpriteFont>
{
public:
	HRESULT Load(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		FontOptions* options, WCHAR* errorMsg, UINT errorLen, SpriteFont** contentOut);
};