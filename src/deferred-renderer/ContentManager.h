#pragma once

#include "PCH.h"
#include "ContentLoader.h"
#include "ContentType.h"
#include "Logger.h"

class ContentManager
{
private:
	typedef std::string LoaderHash;
	typedef std::map<LoaderHash, ContentLoaderBase*> LoaderMap;
	typedef std::map<ContentHash, ContentType*> ContentMap;

	LoaderMap _contentLoaders;
	ContentMap _loadedContent;
	std::wstring _compiledPath;
	std::vector<std::wstring> _searchPaths;

	static const UINT ERROR_MSG_LEN = 1024;

	HRESULT getContentPath(const std::wstring& inPathSegment, std::wstring& outputPath, uint64_t& modDate);
	HRESULT getCompiledPath(const ContentHash& hash, std::wstring& outputPath, bool& available, uint64_t& modDate);
	HRESULT createCompiledContentFolder(const std::wstring& path);

	template <class optionsType, class contentType>
	LoaderHash getContentLoaderHash()
	{
		std::string optionsHash = typeid(optionsType).raw_name();
		std::string contentHash = typeid(contentType).raw_name();
		return optionsHash + contentHash;
	}

	ID3DX11ThreadPump* _threadPump;

public:
	ContentManager();
	~ContentManager();

	void AddContentSearchPath(const std::wstring& path);
	void SetCompiledContentPath(const std::wstring& path);

	template <class optionsType, class contentType>
	void AddContentLoader(ContentLoader<optionsType, contentType>* loader)
	{
		LoaderHash hash = getContentLoaderHash<optionsType, contentType>();
		_contentLoaders[hash] = loader;
	}
	
	template <class optionsType, class contentType>
	HRESULT LoadContent(ID3D11Device* device, const WCHAR* path, optionsType* options,
		contentType** ppContentOut)
	{
		// Find the loader for this content type
		LoaderHash loaderLookupHash = getContentLoaderHash<optionsType, contentType>();
		LoaderMap::iterator loaderIt = _contentLoaders.find(loaderLookupHash);
		if (loaderIt == _contentLoaders.end())
		{
			LOG_ERROR(L"ContentManager", L"Unable find loader for the given types.");
			return E_FAIL;
		}

		// Cast to the loader to the correct type
		ContentLoader<optionsType, contentType>* loader = 
			dynamic_cast<ContentLoader<optionsType, contentType>*>(loaderIt->second);
		if (!loader)
		{
			LOG_ERROR(L"ContentManager", L"Unable to dynamic cast content loader to required type.");
			return E_FAIL;
		}
		
		// Use the loader to generate the content hash
		ContentHash hash;
		if (FAILED(loader->GenerateContentHash(path, options, &hash)))
		{
			LOG_ERROR(L"ContentManager", L"Unable to generate hash of options type.");
			return E_FAIL;
		}


		// If there is no hash, cannot load a stored content or store this content after loading
		if (_loadedContent.find(hash) != _loadedContent.end())
		{
			IUnknown* asContentBase = _loadedContent[hash];

			// Content already loaded, return it
			contentType* asContentType = dynamic_cast<contentType*>(asContentBase);
			if (asContentType)
			{
				asContentType->AddRef();
				*ppContentOut = asContentType;
				return S_OK;
			}
			else
			{
				LOG_ERROR(L"ContentManager", L"Unable to dynamic cast content to required type.");
				return E_FAIL;
			}
		}
		else
		{
			// Generate the full path
			bool contentAvailable = false;
			std::wstring fullPath;
			uint64_t contentModDate;
			if (SUCCEEDED(getContentPath(path, fullPath, contentModDate)))
			{
				contentAvailable = true;
			}

			// Search for a compiled file
			bool compiledAvailable = false;
			std::wstring compiledPath;
			uint64_t compiledModDate;
			if (SUCCEEDED(getCompiledPath(hash, compiledPath, compiledAvailable, compiledModDate)))
			{
				if (compiledAvailable && contentModDate > compiledModDate)
				{
					compiledAvailable = false;
					LOG_INFO(L"ContentManager", L"Found a compiled content file but it is out of date.");
				}
			}
			else
			{
				LOG_ERROR(L"ContentManager", L"Could not generate compiled content file path.");
				return E_FAIL;
			}

			// Load this content
			contentType* content = NULL;
			WCHAR errorMsg[ERROR_MSG_LEN];
			if (contentAvailable && !compiledAvailable)
			{
				createCompiledContentFolder(compiledPath);

				std::ofstream outputStream;
				outputStream.open(compiledPath, std::ios::out | std::ios::binary);
				if (!outputStream.is_open())
				{
					LOG_ERROR(L"ContentManager", L"Could not open output file stream for compiled content.");
					return E_FAIL;
				}

				if (FAILED(loader->CompileContentFile(device, NULL, fullPath.c_str(), options, errorMsg, 
					ERROR_MSG_LEN, &outputStream)))
				{
					LOG_ERROR(L"ContentManager", errorMsg);
					DeleteFile(compiledPath.c_str());
					outputStream.close();
					return E_FAIL;
				}

				outputStream.close();

				compiledAvailable = true;
			}

			if (compiledAvailable)
			{
				std::ifstream inputStream = std::ifstream(compiledPath, std::ios::in | std::ios::binary);
				if (!inputStream.is_open())
				{
					LOG_ERROR(L"ContentManager", L"Could not open input file stream for compiled content.");
					return E_FAIL;
				}

				if (FAILED(loader->LoadFromCompiledContentFile(device, &inputStream, options, errorMsg,
					ERROR_MSG_LEN, &content)))
				{
					LOG_ERROR(L"ContentManager", errorMsg);
					inputStream.close();
					return E_FAIL;
				}

				inputStream.close();

				_loadedContent[hash] = content;
				
				*ppContentOut = content;
				return S_OK;
			}
			else
			{
				LOG_ERROR(L"ContentManager", L"No content files or compiled content available.");
				return E_FAIL;
			}
		}
	}

	template <class contentType>
	HRESULT ReleaseContent(contentType* content)
	{
		ContentType* asContentType = reinterpret_cast<ContentType*>(content);
		if (!content)
		{
			LOG_ERROR(L"ContentManager", L"Attempted to release content object that didn't derive from \
				ContentType");
			return E_FAIL;
		}

		if (asContentType->GetRefCount() > 1)
		{
			asContentType->Release();
			return S_OK;
		}
		else
		{
			for (ContentMap::iterator i = _loadedContent.begin(); i != _loadedContent.end(); i++)
			{
				if (i->second == asContentType)
				{
					_loadedContent.erase(i);
					asContentType->Release();
					return S_OK;
				}
			}

			LOG_ERROR(L"ContentManager", L"Attemped to release content that wasn't held by the content \
										  manager.");
			return E_FAIL;
		}
	}
};