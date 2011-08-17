#pragma once

#include "PCH.h"
#include "ContentLoader.h"
#include "ContentType.h"
#include "Logger.h"

class ContentManager
{
private:
	std::map<long, ContentLoaderBase*> _contentLoaders;
	std::map<long, ContentType*> _loadedContent;
	std::vector<WCHAR*> _searchPaths;

	static const UINT ERROR_MSG_LEN = 512;

	HRESULT getPath(const WCHAR* inPathSegment, WCHAR* outputPath, UINT outputLen);
		
	template <class optionsType, class contentType>
	long getContentLoaderHash()
	{
		size_t optionsHash = typeid(optionsType).hash_code();
		size_t contentHash = typeid(contentType).hash_code();
		return optionsHash + contentHash;
	}

public:
	ContentManager();
	~ContentManager();

	void AddSearchPath(const WCHAR* path);

	template <class optionsType, class contentType>
	void AddContentLoader(ContentLoader<optionsType, contentType>* loader)
	{
		long hash = getContentLoaderHash<optionsType, contentType>();
		_contentLoaders[hash] = loader;
	}
	
	template <class optionsType, class contentType>
	HRESULT LoadContent(ID3D11Device* device, const WCHAR* path, optionsType* options, contentType** ppContentOut)
	{
		// Generate the full path
		WCHAR fullPath[MAX_PATH];
		if (FAILED(getPath(path, fullPath, MAX_PATH)))
		{
			WCHAR errorMsg[MAX_LOG_LENGTH];
			swprintf_s(errorMsg, L"Unable to find file: %s", path);
			LOG_ERROR(L"ContentManager", errorMsg);
			return E_FAIL;
		}

		// Generate hash for this content
		long hash;
		if (FAILED(GenerateContentHash<optionsType>(fullPath, options, &hash)))
		{
			LOG_ERROR(L"ContentManager", L"Unable to generate hash of options type.");
			return E_FAIL;
		}

		if (_loadedContent.find(hash) != _loadedContent.end())
		{
			ContentType* asContentBase = _loadedContent[hash];

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
			// Find the right content loader
			long loaderLookupHash = getContentLoaderHash<optionsType, contentType>();
			if (_contentLoaders.find(loaderLookupHash) == _contentLoaders.end())
			{
				LOG_ERROR(L"ContentManager", L"Unable find loader for the given types.");
				return E_FAIL;
			}

			ContentLoaderBase* asLoaderBase = _contentLoaders[loaderLookupHash];
			ContentLoader<optionsType, contentType>* castedLoader = 
				dynamic_cast<ContentLoader<optionsType, contentType>*>(asLoaderBase);

			if (!castedLoader)
			{
				LOG_ERROR(L"ContentManager", L"Unable to dynamic cast content loader to required type.");
				return E_FAIL;
			}

			// Load this content
			contentType* content = NULL;
			WCHAR errorMsg[ERROR_MSG_LEN];
			while (FAILED(castedLoader->Load(device, NULL, fullPath, options, errorMsg, ERROR_MSG_LEN, &content)))
			{
				LOG_ERROR(L"ContentManager", errorMsg);

				// If in debug mode, display a messagebox saying the content failed to load and give an 
				// option to retry loading
				#if _DEBUG
					WCHAR msgBoxText[ERROR_MSG_LEN];
					swprintf_s(msgBoxText, L"File: %s\nError: %s", fullPath, errorMsg);

					int button = MessageBoxW(NULL, msgBoxText, L"Content loading error", MB_RETRYCANCEL);
					if(button != IDRETRY)
					{
						return E_FAIL;
					}
				#else
					return E_FAIL;
				#endif
			}

			_loadedContent[hash] = content;

			content->AddRef();
			*ppContentOut = content;
			return S_OK;
		}
	}

	void ReleaseContent();
};