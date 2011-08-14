#pragma once

#include "PCH.h"
#include "ContentType.h"

struct ShaderOptions
{
	const char* EntryPoint;
	D3D_SHADER_MACRO* Defines;
};

class PixelShaderContent : public ContentType<ShaderOptions>
{
	ID3D11PixelShader* _ps;

public:
	PixelShaderContent(const WCHAR* path, ShaderOptions* options) : ContentType(path, options) { }

	ID3D11PixelShader* GetPixelShader() { return _ps; }

	HRESULT LoadContent(ID3D11Device* device, const WCHAR* path, ShaderOptions* options);
	void UnloadContent();
};