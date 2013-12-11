#include "PCH.h"
#include "Material.h"
#include "Logger.h"

Material::Material()
    : _ambientColor(0.0f, 0.0f, 0.0f), _diffuseColor(0.0f, 0.0f, 0.0f), _emissiveColor(0.0f, 0.0f, 0.0f),
    _specularColor(0.0f, 0.0f, 0.0f), _specularPower(0.0f), _alpha(1.0f), _diffuseSRV(NULL),
    _normalSRV(NULL), _specularSRV(NULL), _propertiesBuffer(NULL)
{
}

Material::~Material()
{
    Destroy();
}

HRESULT loadCompiledTexture(ID3D11Device* device, std::istream& input, ID3D11ShaderResourceView** output)
{
    UINT texSize;
    BYTE* texData = NULL;
    if (SUCCEEDED(ReadFileFromStream(input, &texData, texSize)) &&
        SUCCEEDED(D3DX11CreateShaderResourceViewFromMemory(device, texData, texSize, NULL, NULL, output, NULL)))
    {
    }
    else
    {
        *output = NULL;
    }

    SAFE_DELETE_ARRAY(texData);
    return S_OK;
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
    SAFE_RELEASE(_diffuseSRV);
    SAFE_RELEASE(_normalSRV);
    SAFE_RELEASE(_specularSRV);
    SAFE_RELEASE(_propertiesBuffer);
}

HRESULT Material::CompileFromSDKMeshMaterial(ID3D11Device* device, const std::wstring& modelDir,
                                             SDKMesh* model, UINT materialIdx, std::ostream& output)
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
    WriteWStringToStream(name, output);
    delete[] name;

    XMFLOAT3 ambientColor = XMFLOAT3(sdkmat->Ambient.x, sdkmat->Ambient.y, sdkmat->Ambient.z);
    WriteDataTostream(ambientColor, output);

    XMFLOAT3 diffuseColor = XMFLOAT3(sdkmat->Diffuse.x, sdkmat->Diffuse.y, sdkmat->Diffuse.z);
    WriteDataTostream(diffuseColor, output);

    XMFLOAT3 specularColor = XMFLOAT3(sdkmat->Specular.x, sdkmat->Specular.y, sdkmat->Specular.z);
    WriteDataTostream(specularColor, output);

    XMFLOAT3 emissiveColor = XMFLOAT3(sdkmat->Emissive.x, sdkmat->Emissive.y, sdkmat->Emissive.z);
    WriteDataTostream(emissiveColor, output);

    WriteDataTostream(sdkmat->Diffuse.w, output); // Alpha
    WriteDataTostream(sdkmat->Power, output);

    if (sdkmat->DiffuseTexture[0] != '\0')
    {
        WriteFileAndSizeToStream(modelDir + AnsiToWString(sdkmat->DiffuseTexture), output);
    }
    else
    {
        WriteFileAndSizeToStream(L"", output);
    }

    if (sdkmat->NormalTexture[0] != '\0')
    {
        WriteFileAndSizeToStream(modelDir + AnsiToWString(sdkmat->NormalTexture), output);
    }
    else
    {
        WriteFileAndSizeToStream(L"", output);
    }

    if (sdkmat->SpecularTexture[0] != '\0')
    {
        WriteFileAndSizeToStream(modelDir + AnsiToWString(sdkmat->SpecularTexture), output);
    }
    else
    {
        WriteFileAndSizeToStream(L"", output);
    }

    return S_OK;
}

HRESULT Material::CompileFromASSIMPMaterial(ID3D11Device* device, const std::wstring& modelDir,
                                            const aiScene* scene, UINT materialIdx, std::ostream& output)
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
    WriteWStringToStream(name, output);
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
    WriteDataTostream(ambientColor, output);

    XMFLOAT3 diffuseColor;
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, col) == aiReturn_SUCCESS)
    {
        diffuseColor = XMFLOAT3(col.r, col.g, col.b);
    }
    else
    {
        diffuseColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
    }
    WriteDataTostream(diffuseColor, output);

    XMFLOAT3 specularColor;
    if (material->Get(AI_MATKEY_SHININESS, col) == aiReturn_SUCCESS)
    {
        specularColor = XMFLOAT3(col.r, col.g, col.b);
    }
    else
    {
        specularColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
    }
    WriteDataTostream(specularColor, output);

    XMFLOAT3 emissiveColor;
    if (material->Get(AI_MATKEY_COLOR_EMISSIVE, col) == aiReturn_SUCCESS)
    {
        emissiveColor = XMFLOAT3(col.r, col.g, col.b);
    }
    else
    {
        emissiveColor = XMFLOAT3(0.0f, 0.0f, 0.0f);
    }
    WriteDataTostream(emissiveColor, output);

    float alpha;
    if (material->Get(AI_MATKEY_OPACITY, alpha) != aiReturn_SUCCESS)
    {
        alpha = 1.0f;
    }
    WriteDataTostream(alpha, output);

    float specularPower;
    if (material->Get(AI_MATKEY_SHININESS_STRENGTH, specularPower) != aiReturn_SUCCESS)
    {
        specularPower = 0.0f;
    }
    WriteDataTostream(specularPower, output);

    // Load textures
    aiString path;

    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0 &&
        material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == aiReturn_SUCCESS)
    {
        WriteFileAndSizeToStream(modelDir + AnsiToWString(path.data), output);
    }
    else
    {
        WriteFileAndSizeToStream(L"", output);
    }

    if (material->GetTextureCount(aiTextureType_NORMALS) > 0 &&
        material->GetTexture(aiTextureType_NORMALS, 0, &path) == aiReturn_SUCCESS)
    {
        WriteFileAndSizeToStream(modelDir + AnsiToWString(path.data), output);
    }
    else
    {
        WriteFileAndSizeToStream(L"", output);
    }

    if (material->GetTextureCount(aiTextureType_SPECULAR) > 0 &&
        material->GetTexture(aiTextureType_SPECULAR, 0, &path) == aiReturn_SUCCESS)
    {
        WriteFileAndSizeToStream(modelDir + AnsiToWString(path.data), output);
    }
    else
    {
        WriteFileAndSizeToStream(L"", output);
    }

    return S_OK;
}

HRESULT Material::Create(ID3D11Device* device, std::istream& input, Material** output)
{
    HRESULT hr;

    Material* result = new Material();

    result->_name = ReadWStringFromStream(input);

    ReadDataFromStream(result->_ambientColor, input);
    ReadDataFromStream(result->_diffuseColor, input);
    ReadDataFromStream(result->_specularColor, input);
    ReadDataFromStream(result->_emissiveColor, input);
    ReadDataFromStream(result->_alpha, input);
    ReadDataFromStream(result->_specularPower, input);

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