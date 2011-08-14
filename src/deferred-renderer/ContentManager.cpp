#include "PCH.h"
#include "ContentManager.h"

ContentManager::ContentManager()
{
	_wgetcwd(_searchPath, MAX_PATH);
}

HRESULT ContentManager::getPath(const WCHAR* inPathSegment, WCHAR* outputPath, UINT outputLen)
{
	wcsncpy_s(outputPath, outputLen, _searchPath, MAX_PATH);
	wcsncat_s(outputPath, outputLen, L"\\", MAX_PATH);
	wcsncat_s(outputPath, outputLen, inPathSegment, MAX_PATH);
	
	// TODO: add error checking
	return S_OK;
}

HRESULT ContentManager::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void ContentManager::OnD3D11DestroyDevice()
{
}

HRESULT ContentManager::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void ContentManager::OnD3D11ReleasingSwapChain()
{
}