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

struct TextureOptions
{
    bool Generate3DFrom2D;
    const char* DebugName;
};

class TextureLoader : public ContentLoader<TextureOptions, TextureContent>
{
public:
    HRESULT GenerateContentHash(const WCHAR* path, TextureOptions* options, ContentHash* hash);
    HRESULT CompileContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump,
        const WCHAR* path, TextureOptions* options, WCHAR* errorMsg, UINT errorLen, std::ostream* output);
    HRESULT LoadFromCompiledContentFile(ID3D11Device* device, std::istream* input, TextureOptions* options,
        WCHAR* errorMsg, UINT errorLen, TextureContent** contentOut);
};