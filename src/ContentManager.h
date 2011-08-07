#pragma once

#include "PCH.h"
#include "IHasContent.h"
#include "ContentType.h"
#include <string>

class ContentManager : public IHasContent
{
private:
	std::map<long, ContentTypeBase*> _contentMap;
	const WCHAR* _searchPath;

	int getPath(const WCHAR* inPathSegment, WCHAR* outputPath, UINT outputLen);

public:
	ContentManager();

	const WCHAR* GetSearchPath() const { return _searchPath; }
	void SetSearchPath(const WCHAR* path) { _searchPath = path; }
	
	template <class contentType>
	HRESULT LoadContent(const WCHAR* path, contentType** ppContentOut)
	{


		return E_NOTIMPL;
	}

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};