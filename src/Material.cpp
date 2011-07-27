#include "Material.h"

Material::Material()
	: _ambientColor(0.0f, 0.0f, 0.0f), _diffuseColor(0.0f, 0.0f, 0.0f), _emissiveColor(0.0f, 0.0f, 0.0f),
	  _specularColor(0.0f, 0.0f, 0.0f), _specularPower(0.0f), _alpha(1.0f), _diffuseSRV(NULL), 
	  _normalSRV(NULL), _specularSRV(NULL)
{
}

Material::~Material()
{
}

HRESULT loadMaterialTexture(ID3D11Device* device, const WCHAR* modelDir, const CHAR* texturePath,
	ID3D11ShaderResourceView** outSRV)
{
	WCHAR wtexturePath[MAX_PATH];
	if (!AnsiToWString(texturePath, wtexturePath, MAX_PATH))
	{
		return E_FAIL;
	}

	WCHAR fullPath[MAX_PATH];
	wcsncpy_s(fullPath, modelDir, MAX_PATH);
	wcsncat_s(fullPath, L"\\", MAX_PATH);
	wcsncat_s(fullPath, wtexturePath, MAX_PATH);

	return D3DX11CreateShaderResourceViewFromFile(device, fullPath, NULL, NULL, outSRV, NULL);
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

HRESULT Material::CreateFromSDKMeshMaterial(ID3D11Device* device, const WCHAR* modelDir, 
	SDKMesh* model, UINT materialIdx)
{
	HRESULT hr;

	SDKMESH_MATERIAL* sdkmat = model->GetMaterial(materialIdx);

	_ambientColor = XMFLOAT3(sdkmat->Ambient.x, sdkmat->Ambient.y, sdkmat->Ambient.z);
	_diffuseColor = XMFLOAT3(sdkmat->Diffuse.x, sdkmat->Diffuse.y, sdkmat->Diffuse.z);
	_specularColor = XMFLOAT3(sdkmat->Specular.x, sdkmat->Specular.y, sdkmat->Specular.z);
	_emissiveColor = XMFLOAT3(sdkmat->Emissive.x, sdkmat->Emissive.y, sdkmat->Emissive.z);
    _alpha = sdkmat->Diffuse.w;
    _specularPower = sdkmat->Power;
	
	if (strlen(sdkmat->DiffuseTexture) > 0 && 
		FAILED(loadMaterialTexture(device, modelDir, sdkmat->DiffuseTexture, &_diffuseSRV)))
	{
		_diffuseSRV = NULL;		
	}

	if (strlen(sdkmat->NormalTexture) > 0 && 
		FAILED(loadMaterialTexture(device, modelDir, sdkmat->NormalTexture, &_normalSRV)))
	{
		_normalSRV = NULL;		
	}

	if (strlen(sdkmat->SpecularTexture) > 0 && 
		FAILED(loadMaterialTexture(device, modelDir, sdkmat->SpecularTexture, &_specularSRV)))
	{
		_specularSRV = NULL;		
	}

	V_RETURN(createPropertiesBuffer(device));

	return S_OK;
}

HRESULT Material::CreateFromASSIMPMaterial(ID3D11Device* device, const WCHAR* modelDir, 
	const aiScene* scene, UINT materialIdx)
{
	HRESULT hr;

	aiMaterial* material = scene->mMaterials[materialIdx];

	// Gather material colors
	aiColor3D col;

	if (material->Get(AI_MATKEY_COLOR_AMBIENT, col) == aiReturn_SUCCESS)
	{
		_ambientColor = XMFLOAT3(col.r, col.g, col.b);
	}
	else
	{
		_ambientColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, col) == aiReturn_SUCCESS)
	{	
		_diffuseColor = XMFLOAT3(col.r, col.g, col.b);
	}
	else
	{
		_diffuseColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
	}

	if (material->Get(AI_MATKEY_SHININESS, col) == aiReturn_SUCCESS)
	{	
		_specularColor = XMFLOAT3(col.r, col.g, col.b);
	}
	else
	{
		_specularColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	if (material->Get(AI_MATKEY_SHININESS_STRENGTH, _specularPower) != aiReturn_SUCCESS)
	{
		_specularPower = 0.0f;
	}

	if (material->Get(AI_MATKEY_COLOR_EMISSIVE, col) == aiReturn_SUCCESS)
	{	
		_emissiveColor = XMFLOAT3(col.r, col.g, col.b);
	}
	else
	{
		_emissiveColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	
	// Load textures
	aiString path;

	_diffuseSRV = NULL;
	if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{		
		material->GetTexture(aiTextureType_DIFFUSE, 0, &path);

		loadMaterialTexture(device, modelDir, path.data, &_diffuseSRV);
	}

	_normalSRV = NULL;
	if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
	{		
		material->GetTexture(aiTextureType_NORMALS, 0, &path);

		loadMaterialTexture(device, modelDir, path.data, &_normalSRV);
	}

	_specularSRV = NULL;
	if (material->GetTextureCount(aiTextureType_SPECULAR) > 0)
	{		
		material->GetTexture(aiTextureType_SPECULAR, 0, &path);

		loadMaterialTexture(device, modelDir, path.data, &_specularSRV);
	}

	V_RETURN(createPropertiesBuffer(device));

	return S_OK;
}

void Material::Destroy()
{
	SAFE_RELEASE(_diffuseSRV);
	SAFE_RELEASE(_normalSRV);
	SAFE_RELEASE(_specularSRV);
	SAFE_RELEASE(_propertiesBuffer);
}