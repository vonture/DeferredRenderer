#include "PCH.h"
#include "ContentManager.h"

ContentManager::ContentManager()
	: _compiledPath(L"")
{
	HRESULT hr;
	V(D3DX11CreateThreadPump(0, 0, &_threadPump)); 
}

ContentManager::~ContentManager()
{
}

HRESULT ContentManager::getContentPath(const std::wstring& inPathSegment, std::wstring& outputPath,
	uint64_t& modDate)
{
	WIN32_FILE_ATTRIBUTE_DATA attrData;

	// See if the segment is a full path
	if (GetFileAttributesEx(inPathSegment.c_str(), GetFileExInfoStandard, &attrData))
	{
		outputPath = inPathSegment;

		modDate = attrData.ftLastWriteTime.dwHighDateTime;
		modDate = (modDate << 32) + attrData.ftLastWriteTime.dwLowDateTime;

		return S_OK;
	}

	// Scan the search paths
	for (UINT i = 0; i < _searchPaths.size(); i++)
	{
		outputPath = _searchPaths[i] + std::wstring(L"\\") + inPathSegment;

		if (GetFileAttributesEx(outputPath.c_str(), GetFileExInfoStandard, &attrData))
		{
			modDate = attrData.ftLastWriteTime.dwHighDateTime;
			modDate = (modDate << 32) + attrData.ftLastWriteTime.dwLowDateTime;

			return S_OK;
		}
	}

	return E_FAIL;
}

HRESULT ContentManager::getCompiledPath(const ContentHash& hash, std::wstring& outputPath, 
	bool& available, uint64_t& modDate)
{
	outputPath = _compiledPath + std::wstring(L"\\") + hash;

	WIN32_FILE_ATTRIBUTE_DATA attrData;
	if (GetFileAttributesEx(outputPath.c_str(), GetFileExInfoStandard, &attrData))
	{
		modDate = attrData.ftLastWriteTime.dwHighDateTime;
		modDate = (modDate << 32) + attrData.ftLastWriteTime.dwLowDateTime;
		available = true;
	}
	else
	{
		available = false;
	}
	return S_OK;
}

HRESULT ContentManager::createCompiledContentFolder(const std::wstring& path)
{
	std::wstring folder = GetDirectoryFromFileNameW(path);
	if (SHCreateDirectoryEx(NULL, folder.c_str(), NULL) == ERROR_SUCCESS)
	{
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

void ContentManager::AddContentSearchPath(const std::wstring& path)
{
	wchar_t* cwd = _wgetcwd(NULL, 0);
	_searchPaths.push_back(std::wstring(cwd) + path);
	free(cwd);
}

void ContentManager::SetCompiledContentPath(const std::wstring& path)
{
	wchar_t* cwd = _wgetcwd(NULL, 0);
	_compiledPath = std::wstring(cwd) + path;
	free(cwd);
}