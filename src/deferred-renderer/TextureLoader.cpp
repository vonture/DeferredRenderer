#include "PCH.h"
#include "TextureLoader.h"
#include "Logger.h"
#include "DDSTextureLoader.h"

HRESULT TextureLoader::GenerateContentHash(const WCHAR* path, TextureOptions* options, ContentHash* hash)
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

HRESULT TextureLoader::LoadFromCompiledContentFile(ID3D11Device* device, std::istream* input,
	TextureOptions* options, WCHAR* errorMsg, UINT errorLen, TextureContent** contentOut)
{
	HRESULT hr;

	UINT size;
	if (!input->read((char*)&size, sizeof(UINT)))
	{
		return E_FAIL;
	}

	BYTE* data = new BYTE[size];
	if (!input->read((char*)data, size))
	{
		return E_FAIL;
	}

	TextureContent* content = new TextureContent();

	hr = D3DX11GetImageInfoFromMemory(data, size, NULL, &content->Info, NULL);
	if (FAILED(hr))
	{
		FormatDXErrorMessageW(hr, errorMsg, errorLen);
		return hr;
	}

	if (options && options->Generate3DFrom2D)
	{
		hr = CreateDDSTexture3DFromMemory(device, data, size, &content->ShaderResourceView);
	}
	else
	{
		hr = D3DX11CreateShaderResourceViewFromMemory(device, data, size, NULL, NULL,
			&content->ShaderResourceView, NULL);
	}
	if (FAILED(hr))
	{
		FormatDXErrorMessageW(hr, errorMsg, errorLen);
		return hr;
	}

	if (options && options->DebugName)
	{
		CHAR debugName[256];

		sprintf_s(debugName, "%s %s", options->DebugName, "SRV");
		V_RETURN(SetDXDebugName(content->ShaderResourceView, debugName));
	}
	*contentOut = content;
	return S_OK;
}

HRESULT TextureLoader::CompileContentFile(ID3D11Device* device, ID3DX11ThreadPump* threadPump, 
	const WCHAR* path, TextureOptions* options, WCHAR* errorMsg, UINT errorLen, std::ostream* output)
{
	std::ifstream file;
	file.open(path, std::ios::in | std::ios::binary);
	if(!file.is_open())
	{
		E_FAIL;
	}

	file.seekg(0, ios::end);
	UINT fileSize = file.tellg();
	file.seekg(0, ios::beg);

	if (fileSize > 0)
	{
		BYTE* data = new BYTE[fileSize];
		file.read((char*)data, fileSize);

		if (!output->write((const char*)&fileSize, sizeof(UINT)))
		{
			SAFE_DELETE_ARRAY(data);
			return E_FAIL;
		}

		if (!output->write((const char*)data, fileSize))
		{
			SAFE_DELETE_ARRAY(data);
			return E_FAIL;
		}

		SAFE_DELETE_ARRAY(data);
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}
