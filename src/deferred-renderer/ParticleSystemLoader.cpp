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

HRESULT ParticleSystemLoader::LoadFromContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump, const WCHAR* path, 
	ParticleSystemOptions* options, WCHAR* errorMsg, UINT errorLen, ParticleSystem** contentOut)
{
	WCHAR logMsg[MAX_LOG_LENGTH];
	swprintf_s(logMsg, L"Loading - %s", path);
	LOG_INFO(L"Particle System Loader", logMsg);

	ParticleSystem* system = new ParticleSystem();

	HRESULT hr = system->CreateFromFile(device, path);
	if (FAILED(hr))
	{
		delete system;
		return hr;
	}

	*contentOut = system;
	return S_OK;
}

HRESULT ParticleSystemLoader::LoadFromCompiledContentFile(ID3D11Device* device, const WCHAR* path, WCHAR* errorMsg,
	UINT errorLen, ParticleSystem** contentOut)
{
	return E_NOTIMPL;
}