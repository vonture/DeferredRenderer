#pragma once

#include "PCH.h"
#include "ContentLoader.h"
#include "ContentType.h"

struct GeometryShaderContent : public ContentType
{
    ID3D11GeometryShader* GeometryShader;

    GeometryShaderContent() : GeometryShader(NULL) { }
    ~GeometryShaderContent() { SAFE_RELEASE(GeometryShader); }
};

struct GeometryShaderOptions
{
    const char* EntryPoint;
    D3D_SHADER_MACRO* Defines;
    const char* DebugName;
};

class GeometryShaderLoader : public ContentLoader<GeometryShaderOptions, GeometryShaderContent>
{
public:
    HRESULT GenerateContentHash(const WCHAR* path, GeometryShaderOptions* options, ContentHash* hash);
    HRESULT CompileContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump,
        const WCHAR* path, GeometryShaderOptions* options, WCHAR* errorMsg, UINT errorLen, std::ostream* output);
    HRESULT LoadFromCompiledContentFile(ID3D11Device* device, std::istream* input, GeometryShaderOptions* options,
        WCHAR* errorMsg, UINT errorLen, GeometryShaderContent** contentOut);
};