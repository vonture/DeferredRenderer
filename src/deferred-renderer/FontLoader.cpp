#include "PCH.h"
#include "FontLoader.h"

HRESULT FontLoader::GenerateContentHash(const WCHAR* path, FontOptions* options, ContentHash* hash)
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

HRESULT FontLoader::LoadFromCompiledContentFile(ID3D11Device* device, std::istream* input, 
	FontOptions* options, WCHAR* errorMsg, UINT errorLen, SpriteFont** contentOut)
{
	return SpriteFont::Create(device, input, contentOut);
}

HRESULT FontLoader::CompileContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump,
	const WCHAR* path, FontOptions* options, WCHAR* errorMsg, UINT errorLen, std::ostream* output)
{
	return SpriteFont::Compile(device, path, output);
}
