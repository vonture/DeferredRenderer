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
	WCHAR _searchPath[MAX_PATH];

	static const UINT ERROR_MSG_LEN = 256;

	HRESULT getPath(const WCHAR* inPathSegment, WCHAR* outputPath, UINT outputLen);

public:
	ContentManager();

	const WCHAR* GetSearchPath() const { return _searchPath; }
	void SetSearchPath(const WCHAR* path) { wcscpy_s(_searchPath, path); }
	
	template <class optionsType, class contentType>
	HRESULT LoadContent(ID3D11Device* device, const WCHAR* path, optionsType* options, contentType** ppContentOut)
	{
		// Generate hash for this content
		long hash;
		if (FAILED(GenerateContentHash<optionsType>(path, options, &hash)))
		{
			LOG_ERROR(L"ContentManager", L"Unable to generate hash of options type.");
			return E_FAIL;
		}

		if (_loadedContent.find(hash) != _contentMap.end())
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
			size_t optionsHash = typeid(lightType).hash_code();
			size_t contentHash = typeid(contentType).hash_code();
			long loaderLookupHash = optionsHash + contentHash;

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
			while (FAILED(castedLoader->Load(device, NULL, path, options, errorMsg, ERROR_MSG_LEN, &content)))
			{
				LOG_ERROR(L"ContentManager", errorMsg);

				// If in debug mode, display a messagebox saying the content failed to load and give an 
				// option to retry loading
				#if _DEBUG
					int button = MessageBoxW(NULL, errorMsg, L"Content loading error", MB_RETRYCANCEL);
					if(button != IDRETRY)
					{
						return E_FAIL;
					}
				#else
					return E_FAIL;
				#endif
			}

			_loadedContent[hash] = content;

			asContentType->AddRef();
			*ppContentOut = asContentType;
			return S_OK;
		}
	}

	void ReleaseContent();
};