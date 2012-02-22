#include "PCH.h"
#include "ParticleSystemLoader.h"
#include "Logger.h"

HRESULT ParticleSystemLoader::GenerateContentHash(const WCHAR* path, ParticleSystemOptions* options, ContentHash* hash)
{
	if (!hash)
	{
		return FWP_E_NULL_POINTER;
	}

	ContentHash retHash;
	retHash.append(path);

	*hash = retHash;
	return S_OK;
}

HRESULT ParticleSystemLoader::LoadFromCompiledContentFile(ID3D11Device* device, std::istream* input,
	ParticleSystemOptions* options, WCHAR* errorMsg, UINT errorLen, ParticleSystem** contentOut)
{
	return ParticleSystem::Create(device, *input, contentOut);
}

HRESULT ParticleSystemLoader::CompileContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump,
	const WCHAR* path, ParticleSystemOptions* options, WCHAR* errorMsg, UINT errorLen, std::ostream* output)
{
	return ParticleSystem::Compile(device, path, *output);
}
