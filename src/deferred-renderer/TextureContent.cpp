#include "PCH.h"
#include "TextureContent.h"

HRESULT Texture2DContent::LoadContent(ID3D11Device* device, const WCHAR* path)
{
	HRESULT hr;

	V_RETURN(D3DX11GetImageInfoFromFile(path, NULL, &_info, NULL));
	V_RETURN(D3DX11CreateShaderResourceViewFromFile(device, path, NULL, NULL, &_srv, NULL));

	return S_OK;
}

void Texture2DContent::UnloadContent()
{
	SAFE_RELEASE(_srv);
}