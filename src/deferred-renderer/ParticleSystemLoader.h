#pragma once

#include "PCH.h"
#include "ContentLoader.h"
#include "ContentType.h"
#include "ParticleSystem.h"

struct ParticleSystemOptions
{
};

class ParticleSystemLoader : public ContentLoader<ParticleSystemOptions, ParticleSystem>
{
public:
	HRESULT GenerateContentHash(const WCHAR* path, ParticleSystemOptions* options, ContentHash* hash);
	HRESULT LoadFromContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
		ParticleSystemOptions* options, WCHAR* errorMsg, UINT errorLen, ParticleSystem** contentOut);
	HRESULT LoadFromCompiledContentFile(ID3D11Device* device, const WCHAR* path, WCHAR* errorMsg,
		UINT errorLen, ParticleSystem** contentOut);
};