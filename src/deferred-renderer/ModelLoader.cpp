#include "PCH.h"
#include "ModelLoader.h"
#include "Logger.h"

HRESULT ModelLoader::GenerateContentHash(const WCHAR* path, ModelOptions* options, ContentHash* hash)
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

HRESULT ModelLoader::LoadFromCompiledContentFile(ID3D11Device* device, std::istream* input,
	ModelOptions* options, WCHAR* errorMsg, UINT errorLen, Model** contentOut)
{
	return Model::Create(device, input, contentOut);
}

HRESULT ModelLoader::CompileContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump,
	const WCHAR* path, ModelOptions* options, WCHAR* errorMsg, UINT errorLen, std::ostream* output)
{
	return Model::Compile(device, path, output);
}
