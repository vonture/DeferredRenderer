#include "PCH.h"
#include "ContentManager.h"

ContentManager::ContentManager()
{
	SetSearchPath(L"");
}

HRESULT ContentManager::getPath(const WCHAR* inPathSegment, WCHAR* outputPath, UINT outputLen)
{
	wcsncpy_s(outputPath, outputLen, _searchPath, MAX_PATH);
	wcsncat_s(outputPath, outputLen, L"\\", MAX_PATH);
	wcsncat_s(outputPath, outputLen, inPathSegment, MAX_PATH);
	
	// TODO: add error checking
	return S_OK;
}

void ContentManager::SetSearchPath(const WCHAR* path)
{
	_wgetcwd(_searchPath, MAX_PATH);
	wcsncat_s(_searchPath, MAX_PATH, L"\\", MAX_PATH);
	wcsncat_s(_searchPath, MAX_PATH, path, MAX_PATH);
}

void ContentManager::ReleaseContent()
{
	std::map<long, ContentType*>::iterator i;
	for (i = _loadedContent.begin(); i != _loadedContent.end(); i++)
	{
		SAFE_RELEASE(i->second);
	}

	_loadedContent.clear();
}