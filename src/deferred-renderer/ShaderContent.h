#pragma once

#include "PCH.h"
#include "ContentType.h"

struct PixelShaderContent : public ContentType
{
	ID3D11PixelShader* PixelShader;

	PixelShaderContent() : PixelShader(NULL) { }
	~PixelShaderContent() { SAFE_RELEASE(PixelShader); }
};