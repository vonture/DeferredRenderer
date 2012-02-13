#include "PCH.h"
#include "Material.h"
#include "Logger.h"

Material::Material()
	: _ambientColor(0.0f, 0.0f, 0.0f), _diffuseColor(0.0f, 0.0f, 0.0f), _emissiveColor(0.0f, 0.0f, 0.0f),
	  _specularColor(0.0f, 0.0f, 0.0f), _specularPower(0.0f), _alpha(1.0f), _diffuseSRV(NULL), 
	  _normalSRV(NULL), _specularSRV(NULL), _name(NULL), _propertiesBuffer(NULL)
{
}

Material::~Material()
{
	Destroy();
}

HRESULT loadMaterialTexture(ID3D11Device* device, const WCHAR* modelDir, const CHAR* texturePath,
	ID3D11ShaderResourceView** outSRV, std::map<TexturePathHash, ID3D11ShaderResourceView*>* loadedTextureMap = NULL)
{
	HRESULT hr;

	WCHAR wtexturePath[MAX_PATH];
	if (!AnsiToWString(texturePath, wtexturePath, MAX_PATH))
	{
		return E_FAIL;
	}

	std::wstring fullPath = std::wstring(modelDir);
	fullPath += L"\\";
	fullPath += wtexturePath;
		
	if (loadedTextureMap)
	{
		std::map<TexturePathHash, ID3D11ShaderResourceView*>::iterator it = loadedTextureMap->find(fullPath);
		if (it != loadedTextureMap->end())
		{
			*outSRV = it->second;
			(*outSRV)->AddRef();

			return S_OK;
		}
	}
		
	hr = D3DX11CreateShaderResourceViewFromFile(device, fullPath.c_str(), NULL, NULL, outSRV, NULL);
	if (loadedTextureMap && SUCCEEDED(hr))
	{
		loadedTextureMap->insert(std::pair<TexturePathHash, ID3D11ShaderResourceView*>(fullPath, *outSRV));
	}

	return hr;
}

HRESULT loadMaterialTextureToMemory(ID3D11Device* device, const WCHAR* modelDir, const CHAR* texturePath,
	BYTE** output, int* outputSize)
{

	WCHAR wtexturePath[MAX_PATH];
	if (!AnsiToWString(texturePath, wtexturePath, MAX_PATH))
	{
		return E_FAIL;
	}

	std::wstring fullPath = std::wstring(modelDir);
	fullPath += L"\\";
	fullPath += wtexturePath;

	std::ifstream file;
	file.open(fullPath, std::ios::in | std::ios::binary);

	if(!file.is_open())
	{
		E_FAIL;
	}

	file.seekg(0, ios::end);
	std::streamoff fileSize = file.tellg();
	file.seekg(0, ios::beg);

	if (fileSize > 0)
	{
		*outputSize = fileSize;
		*output = new BYTE[fileSize];

		file.read((char *)*output, fileSize);

		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

HRESULT loadCompiledTexture(ID3D11Device* device, std::istream* input, ID3D11ShaderResourceView** output)
{
	HRESULT hr;

	// Read the length
	UINT texSize;
	input->read((char*)&texSize, sizeof(int));

	if (texSize > 0)
	{
		BYTE* texData = new BYTE[texSize];
		input->read((char*)texData, texSize);

		hr = D3DX11CreateShaderResourceViewFromMemory(device, texData, texSize, NULL, NULL, output, NULL);
	
		delete[] texData;

		return hr;
	}
	else
	{
		*output = NULL;
		return S_OK;
	}
}

HRESULT Material::createPropertiesBuffer(ID3D11Device* device)
{
	// Create the buffer
	D3D11_BUFFER_DESC bufferDesc =
	{
		sizeof(CB_MATERIAL_PROPERTIES), //UINT ByteWidth;
		D3D11_USAGE_DYNAMIC, //D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER, //UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, //UINT CPUAccessFlags;
		0, //UINT MiscFlags;
		0, //UINT StructureByteStride;
	};

	CB_MATERIAL_PROPERTIES bufferData;
	bufferData.AmbientColor = _ambientColor;
	bufferData.DiffuseColor = _diffuseColor;
	bufferData.EmissiveColor = _emissiveColor;
	bufferData.SpecularColor = _specularColor;
	bufferData.SpecularPower = _specularPower;
	bufferData.Alpha = _alpha;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &bufferData;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	return device->CreateBuffer(&bufferDesc, &initData, &_propertiesBuffer);
}

void Material::Destroy()
{
	SAFE_DELETE(_name);
	SAFE_RELEASE(_diffuseSRV);
	SAFE_RELEASE(_normalSRV);
	SAFE_RELEASE(_specularSRV);
	SAFE_RELEASE(_propertiesBuffer);
}

HRESULT Material::CompileFromSDKMeshMaterial(ID3D11Device* device, const WCHAR* modelDir,
	SDKMesh* model, UINT materialIdx, std::ostream* output)
{
	SDKMESH_MATERIAL* sdkmat = model->GetMaterial(materialIdx);

	// Copy the name
	UINT nameLen = strlen(sdkmat->Name);
	WCHAR* name = NULL;
	if (nameLen > 0)
	{
		name = new WCHAR[nameLen + 1];
		AnsiToWString(sdkmat->Name, name, nameLen + 1);
	}
	else
	{
		name = new WCHAR[MAX_PATH];
		swprintf_s(name, MAX_PATH, L"Material %u", materialIdx);
	}

	UINT finalNameLen = wcslen(name);
	if (!output->write((const char*)&finalNameLen, sizeof(UINT)))
	{
		delete[] name;
		return E_FAIL;
	}
	if (!output->write((const char*)name, sizeof(WCHAR) * finalNameLen))
	{
		delete[] name;
		return E_FAIL;
	}
	delete[] name;
	
	XMFLOAT3 ambientColor = XMFLOAT3(sdkmat->Ambient.x, sdkmat->Ambient.y, sdkmat->Ambient.z);
	if (!output->write((const char*)&ambientColor, sizeof(XMFLOAT3)))
	{
		return E_FAIL;
	}
	
	XMFLOAT3 diffuseColor = XMFLOAT3(sdkmat->Diffuse.x, sdkmat->Diffuse.y, sdkmat->Diffuse.z);
	if (!output->write((const char*)&diffuseColor, sizeof(XMFLOAT3)))
	{
		return E_FAIL;
	}

	XMFLOAT3 specularColor = XMFLOAT3(sdkmat->Specular.x, sdkmat->Specular.y, sdkmat->Specular.z);
	if (!output->write((const char*)&specularColor, sizeof(XMFLOAT3)))
	{
		return E_FAIL;
	}

	XMFLOAT3 emissiveColor = XMFLOAT3(sdkmat->Emissive.x, sdkmat->Emissive.y, sdkmat->Emissive.z);
	if (!output->write((const char*)&emissiveColor, sizeof(XMFLOAT3)))
	{
		return E_FAIL;
	}

	float alpha = sdkmat->Diffuse.w;
	if (!output->write((const char*)&alpha, sizeof(float)))
	{
		return E_FAIL;
	}

	float specularPower = sdkmat->Power;
	if (!output->write((const char*)&specularPower, sizeof(float)))
	{
		return E_FAIL;
	}
		
	BYTE* textureData = NULL;
	int textureSize = 0;
	int emptyTextureSize = 0;

	if (strlen(sdkmat->DiffuseTexture) && 
		SUCCEEDED(loadMaterialTextureToMemory(device, modelDir, sdkmat->DiffuseTexture, &textureData, &textureSize)))
	{
		if (!output->write((const char*)&textureSize, sizeof(int)))
		{
			SAFE_DELETE_ARRAY(textureData);
			return E_FAIL;
		}
		if (!output->write((const char*)textureData, textureSize))
		{
			SAFE_DELETE_ARRAY(textureData);
			return E_FAIL;
		}
		SAFE_DELETE_ARRAY(textureData);
	}
	else
	{
		if (!output->write((const char*)&emptyTextureSize, sizeof(int)))
		{
			return E_FAIL;
		}
	}
	
	if (strlen(sdkmat->NormalTexture) && 
		SUCCEEDED(loadMaterialTextureToMemory(device, modelDir, sdkmat->NormalTexture, &textureData, &textureSize)))
	{
		if (!output->write((const char*)&textureSize, sizeof(int)))
		{
			SAFE_DELETE_ARRAY(textureData);
			return E_FAIL;
		}
		if (!output->write((const char*)textureData, textureSize))
		{
			SAFE_DELETE_ARRAY(textureData);
			return E_FAIL;
		}
		SAFE_DELETE_ARRAY(textureData);
	}
	else
	{
		if (!output->write((const char*)&emptyTextureSize, sizeof(int)))
		{
			return E_FAIL;
		}
	}

	if (strlen(sdkmat->SpecularTexture) && 
		SUCCEEDED(loadMaterialTextureToMemory(device, modelDir, sdkmat->SpecularTexture, &textureData, &textureSize)))
	{
		if (!output->write((const char*)&textureSize, sizeof(int)))
		{
			SAFE_DELETE_ARRAY(textureData);
			return E_FAIL;
		}
		if (!output->write((const char*)textureData, textureSize))
		{
			SAFE_DELETE_ARRAY(textureData);
			return E_FAIL;
		}
		SAFE_DELETE_ARRAY(textureData);
	}
	else
	{
		if (!output->write((const char*)&emptyTextureSize, sizeof(int)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT Material::CompileFromASSIMPMaterial(ID3D11Device* device, const WCHAR* modelDir,
	const aiScene* scene, UINT materialIdx, std::ostream* output)
{
	aiMaterial* material = scene->mMaterials[materialIdx];

	// Get the name
	aiString assimpName;
	WCHAR* name = NULL;
	if (material->Get(AI_MATKEY_NAME, assimpName) == aiReturn_SUCCESS && assimpName.length > 0)
	{
		name = new WCHAR[assimpName.length + 1];
		AnsiToWString(assimpName.data, name, assimpName.length + 1);
	}
	else
	{
		name = new WCHAR[MAX_PATH];
		swprintf_s(name, MAX_PATH, L"Material %u", materialIdx);
	}

	UINT nameLen = wcslen(name);
	if (!output->write((const char*)&nameLen, sizeof(UINT)))
	{
		delete[] name;
		return E_FAIL;
	}
	if (!output->write((const char*)name, sizeof(WCHAR) * nameLen))
	{
		delete[] name;
		return E_FAIL;
	}
	delete[] name;

	// Gather material colors
	aiColor3D col;

	XMFLOAT3 ambientColor;
	if (material->Get(AI_MATKEY_COLOR_AMBIENT, col) == aiReturn_SUCCESS)
	{
		ambientColor = XMFLOAT3(col.r, col.g, col.b);
	}
	else
	{
		ambientColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	if (!output->write((const char*)&ambientColor, sizeof(XMFLOAT3)))
	{
		return E_FAIL;
	}

	XMFLOAT3 diffuseColor;
	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, col) == aiReturn_SUCCESS)
	{	
		diffuseColor = XMFLOAT3(col.r, col.g, col.b);
	}
	else
	{
		diffuseColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
	}
	if (!output->write((const char*)&diffuseColor, sizeof(XMFLOAT3)))
	{
		return E_FAIL;
	}

	XMFLOAT3 specularColor;
	if (material->Get(AI_MATKEY_SHININESS, col) == aiReturn_SUCCESS)
	{	
		specularColor = XMFLOAT3(col.r, col.g, col.b);
	}
	else
	{
		specularColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	if (!output->write((const char*)&specularColor, sizeof(XMFLOAT3)))
	{
		return E_FAIL;
	}

	XMFLOAT3 emissiveColor;
	if (material->Get(AI_MATKEY_COLOR_EMISSIVE, col) == aiReturn_SUCCESS)
	{	
		emissiveColor = XMFLOAT3(col.r, col.g, col.b);
	}
	else
	{
		emissiveColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	if (!output->write((const char*)&emissiveColor, sizeof(XMFLOAT3)))
	{
		return E_FAIL;
	}

	float alpha;
	if (material->Get(AI_MATKEY_OPACITY, alpha) != aiReturn_SUCCESS)
	{
		alpha = 1.0f;
	}
	if (!output->write((const char*)&alpha, sizeof(float)))
	{
		return E_FAIL;
	}

	float specularPower;
	if (material->Get(AI_MATKEY_SHININESS_STRENGTH, specularPower) != aiReturn_SUCCESS)
	{
		specularPower = 0.0f;
	}
	if (!output->write((const char*)&specularPower, sizeof(float)))
	{
		return E_FAIL;
	}

	// Load textures
	aiString path;
	
	BYTE* textureData = NULL;
	int textureSize = 0;
	int emptyTextureSize = 0;

	if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0 &&
		material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == aiReturn_SUCCESS &&
		SUCCEEDED(loadMaterialTextureToMemory(device, modelDir, path.data, &textureData, &textureSize)))
	{
		if (!output->write((const char*)&textureSize, sizeof(int)))
		{
			SAFE_DELETE_ARRAY(textureData);
			return E_FAIL;
		}
		if (!output->write((const char*)textureData, textureSize))
		{
			SAFE_DELETE_ARRAY(textureData);
			return E_FAIL;
		}
		SAFE_DELETE_ARRAY(textureData);
	}
	else
	{
		if (!output->write((const char*)&emptyTextureSize, sizeof(int)))
		{
			return E_FAIL;
		}
	}

	if (material->GetTextureCount(aiTextureType_NORMALS) > 0 &&
		material->GetTexture(aiTextureType_NORMALS, 0, &path) == aiReturn_SUCCESS &&
		SUCCEEDED(loadMaterialTextureToMemory(device, modelDir, path.data, &textureData, &textureSize)))
	{
		if (!output->write((const char*)&textureSize, sizeof(int)))
		{
			SAFE_DELETE_ARRAY(textureData);
			return E_FAIL;
		}
		if (!output->write((const char*)textureData, textureSize))
		{
			SAFE_DELETE_ARRAY(textureData);
			return E_FAIL;
		}
		SAFE_DELETE_ARRAY(textureData);
	}
	else
	{
		if (!output->write((const char*)&emptyTextureSize, sizeof(int)))
		{
			return E_FAIL;
		}
	}

	if (material->GetTextureCount(aiTextureType_SPECULAR) > 0 &&
		material->GetTexture(aiTextureType_SPECULAR, 0, &path) == aiReturn_SUCCESS &&
		SUCCEEDED(loadMaterialTextureToMemory(device, modelDir, path.data, &textureData, &textureSize)))
	{
		if (!output->write((const char*)&textureSize, sizeof(int)))
		{
			SAFE_DELETE_ARRAY(textureData);
			return E_FAIL;
		}
		if (!output->write((const char*)textureData, textureSize))
		{
			SAFE_DELETE_ARRAY(textureData);
			return E_FAIL;
		}
		SAFE_DELETE_ARRAY(textureData);
	}
	else
	{
		if (!output->write((const char*)&emptyTextureSize, sizeof(int)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT Material::Create(ID3D11Device* device, std::istream* input, Material** output)
{
	HRESULT hr;

	Material* result = new Material();

	UINT nameLen;
	input->read((char*)&nameLen, sizeof(UINT));

	result->_name = new WCHAR[nameLen + 1];
	result->_name[nameLen] = '\0';
	input->read((char*)result->_name, nameLen * sizeof(WCHAR));

	input->read((char*)&result->_ambientColor, sizeof(XMFLOAT3));
	input->read((char*)&result->_diffuseColor, sizeof(XMFLOAT3));
	input->read((char*)&result->_specularColor, sizeof(XMFLOAT3));
	input->read((char*)&result->_emissiveColor, sizeof(XMFLOAT3));
	input->read((char*)&result->_alpha, sizeof(float));
	input->read((char*)&result->_specularPower, sizeof(float));

	hr = loadCompiledTexture(device, input, &result->_diffuseSRV);
	if (FAILED(hr))
	{
		delete result;
		return hr;
	}

	hr = loadCompiledTexture(device, input, &result->_normalSRV);
	if (FAILED(hr))
	{
		delete result;
		return hr;
	}

	hr = loadCompiledTexture(device, input, &result->_specularSRV);
	if (FAILED(hr))
	{
		delete result;
		return hr;
	}

	hr = result->createPropertiesBuffer(device);
	if (FAILED(hr))
	{
		delete result;
		return hr;
	}

	*output = result;
	return S_OK;
}
