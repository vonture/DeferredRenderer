#pragma once

#include "PCH.h"
#include "IHasContent.h"
#include "ContentType.h"
#include <string>

class ContentManager : public IHasContent
{
private:
	std::map<long, ContentTypeBase*> _contentMap;
	WCHAR _searchPath[MAX_PATH];

	HRESULT getPath(const WCHAR* inPathSegment, WCHAR* outputPath, UINT outputLen);

public:
	ContentManager();

	const WCHAR* GetSearchPath() const { return _searchPath; }
	void SetSearchPath(const WCHAR* path) { wcscpy_s(_searchPath, path); }
	
	template <class T>
	HRESULT LoadContent(const WCHAR* path, T* options, ContentType<T>** ppContentOut)
	{
		return E_NOTIMPL;
	}

	template <class T>
	HRESULT LoadContent(const WCHAR* path, ContentType<T>** ppContentOut)
	{
		return LoadContent<T>(path, NULL, ppContentOut);
	}

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};