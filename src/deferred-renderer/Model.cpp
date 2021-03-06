#include "PCH.h"
#include "Model.h"
#include "SDKmesh.h"

#include "assimp.hpp"      // C++ importer interface
#include "aiScene.h"       // Output data structure
#include "aiPostProcess.h" // Post processing flags
#include "AssimpLogger.h"

Model::Model()
    : _meshes(NULL), _meshCount(0), _materials(NULL), _materialCount(0)
{
}

Model::~Model()
{
    Destroy();
}

IDirect3DDevice9* createD3D9Device()
{
    HRESULT hr;

    // Create a D3D9 device (would make it NULL, but PIX doesn't seem to like that)
    IDirect3D9* d3d9;
    d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

    D3DPRESENT_PARAMETERS pp;
    pp.BackBufferWidth = 1;
    pp.BackBufferHeight = 1;
    pp.BackBufferFormat = D3DFMT_X8R8G8B8;
    pp.BackBufferCount = 1;
    pp.MultiSampleType = D3DMULTISAMPLE_NONE;
    pp.MultiSampleQuality = 0;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = GetDesktopWindow();
    pp.Windowed = true;
    pp.Flags = 0;
    pp.FullScreen_RefreshRateInHz = 0;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    pp.EnableAutoDepthStencil = false;

    IDirect3DDevice9* d3d9Device = NULL;
    V(d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &d3d9Device));

    return d3d9Device;
}

void Model::Destroy()
{
    for (UINT i = 0; i < _materialCount; i++)
    {
        SAFE_RELEASE(_materials[i]);
    }
    SAFE_DELETE_ARRAY(_materials);
    _materialCount = 0;

    for (UINT i = 0; i < _meshCount; i++)
    {
        SAFE_RELEASE(_meshes[i]);
    }
    SAFE_DELETE_ARRAY(_meshes);
    _meshCount = 0;
}

HRESULT Model::Render(ID3D11DeviceContext* context, UINT materialBufferSlot, UINT diffuseSlot,
                      UINT normalSlot, UINT specularSlot)
{
    HRESULT hr;

    for (UINT i = 0; i < _meshCount; i++)
    {
        V_RETURN(RenderMesh(context, i, materialBufferSlot, diffuseSlot, normalSlot, specularSlot));
    }

    return S_OK;
}

HRESULT Model::RenderMesh(ID3D11DeviceContext* context, UINT meshIdx, UINT materialBufferSlot,
                          UINT diffuseSlot, UINT normalSlot, UINT specularSlot)
{
    const Mesh* mesh = _meshes[meshIdx];
    UINT partCount = mesh->GetMeshPartCount();

    ID3D11Buffer* vertexBuffers[1] = { mesh->GetVertexBuffer() };
    UINT strides[1] = { mesh->GetVertexStride() };
    UINT offsets[1] = { 0 };

    context->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
    context->IASetIndexBuffer(mesh->GetIndexBuffer(), mesh->GetIndexBufferFormat(), 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (UINT i = 0; i < partCount; i++)
    {
        const MeshPart* part = mesh->GetMeshPart(i);
        const Material* mat = _materials[part->MaterialIndex];

        if (materialBufferSlot != INVALID_BUFFER_SLOT)
        {
            ID3D11Buffer* buf = mat->GetPropertiesBuffer();
            context->PSSetConstantBuffers(materialBufferSlot, 1, &buf);
        }
        if (diffuseSlot != INVALID_SAMPLER_SLOT)
        {
            ID3D11ShaderResourceView* srv = mat->GetDiffuseSRV();
            context->PSSetShaderResources(diffuseSlot, 1, &srv);
        }
        if (normalSlot != INVALID_SAMPLER_SLOT)
        {
            ID3D11ShaderResourceView* srv = mat->GetNormalSRV();
            context->PSSetShaderResources(normalSlot, 1, &srv);
        }
        if (specularSlot != INVALID_SAMPLER_SLOT)
        {
            ID3D11ShaderResourceView* srv = mat->GetSpecularSRV();
            context->PSSetShaderResources(specularSlot, 1, &srv);
        }

        context->DrawIndexed(part->IndexCount, part->IndexStart, part->VertexStart);
    }

    return S_OK;
}

HRESULT Model::Compile(ID3D11Device* device, const std::wstring& fileName, std::ostream& output)
{
    HRESULT hr;

    std::wstring directory = GetDirectoryFromFileNameW(fileName);
    std::wstring extension = GetExtensionFromFileNameW(fileName);
    std::wstring name = GetFileNameWithoutExtensionW(fileName);

    WriteWStringToStream(name, output);

    Assimp::Importer importer;
    // determine if assimp can import this model
    if (importer.IsExtensionSupported(WStringToAnsi(extension)))
    {
        AssimpLogger::Register();

        UINT importSteps =
            aiProcess_PreTransformVertices            |
            aiProcess_ConvertToLeftHanded            |
            aiProcess_CalcTangentSpace                |
            aiProcess_GenSmoothNormals                |
            aiProcess_JoinIdenticalVertices            |
            aiProcess_ImproveCacheLocality            |
            aiProcess_LimitBoneWeights                |
            aiProcess_RemoveRedundantMaterials      |
            aiProcess_SplitLargeMeshes                |
            aiProcess_Triangulate                    |
            aiProcess_GenUVCoords                   |
            aiProcess_SortByPType                   |
            aiProcess_FindDegenerates               |
            aiProcess_FindInstances                 |
            aiProcess_ValidateDataStructure         |
            aiProcess_OptimizeMeshes;

        importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE |
            aiPrimitiveType_POLYGON);

        // Load with assimp
        const aiScene* scene = importer.ReadFile(WStringToAnsi(fileName), importSteps);
        if (!scene)
        {
            std::wstring error = AnsiToWString(importer.GetErrorString());
            LOG_ERROR(L"Model Load", error);

            return E_FAIL;
        }

        UINT materialCount = scene->mNumMaterials;
        WriteDataTostream(materialCount, output);
        for (UINT i = 0; i < materialCount; i++)
        {
            V_RETURN(Material::CompileFromASSIMPMaterial(device, directory, scene, i, output));
        }

        UINT meshCount = scene->mNumMeshes;
        WriteDataTostream(meshCount, output);
        for (UINT i = 0; i < meshCount; i++)
        {
            V_RETURN(Mesh::CompileFromASSIMPMesh(device, scene, i, output));
        }
    }
    else if (extension == L".x" || extension == L".sdkmesh")
    {
        // load with sdkmesh
        SDKMesh sdkMesh;
        V_RETURN(sdkMesh.Create(fileName.c_str()));

        // Make materials
        UINT materialCount = sdkMesh.GetNumMaterials();
        WriteDataTostream(materialCount, output);
        for (UINT i = 0; i < materialCount; i++)
        {
            hr = Material::CompileFromSDKMeshMaterial(device, directory, &sdkMesh, i, output);
            if (FAILED(hr))
            {
                sdkMesh.Destroy();
                return hr;
            }
        }

        // Create a d3d9 device for loading the meshes
        IDirect3DDevice9* d3d9device = createD3D9Device();

        // Copy the meshes
        UINT meshCount = sdkMesh.GetNumMeshes();
        WriteDataTostream(meshCount, output);
        for (UINT i = 0; i < meshCount; i++)
        {
            hr = Mesh::CompileFromSDKMeshMesh(device, d3d9device, directory, &sdkMesh, i, output);
            if (FAILED(hr))
            {
                SAFE_RELEASE(d3d9device);
                sdkMesh.Destroy();
                return hr;
            }
        }

        SAFE_RELEASE(d3d9device);
        sdkMesh.Destroy();
    }
    else
    {
        // Don't know how to load this model
        return E_FAIL;
    }

    return S_OK;
}

HRESULT Model::Create(ID3D11Device* device, std::istream& input, Model** output)
{
    HRESULT hr;

    Model* result = new Model();

    result->_name = ReadWStringFromStream(input);

    ReadDataFromStream(result->_materialCount, input);
    result->_materials = new Material*[result->_materialCount];
    // NULL everything first so that if there is an error, the model destructor wont
    // reference an uninitialized value
    memset(result->_materials, NULL, sizeof(Material*) * result->_materialCount);
    for (UINT i = 0; i < result->_materialCount; i++)
    {
        result->_materials[i] = new Material();

        hr = Material::Create(device, input, &result->_materials[i]);
        if (FAILED(hr))
        {
            SAFE_RELEASE(result);
            return hr;
        }
    }

    ReadDataFromStream(result->_meshCount, input);

    result->_meshes = new Mesh*[result->_meshCount];
    memset(result->_meshes, NULL, sizeof(Mesh*) * result->_meshCount);
    for (UINT i = 0; i < result->_meshCount; i++)
    {
        result->_meshes[i] = new Mesh();

        hr = Mesh::Create(device, input, &result->_meshes[i]);
        if (FAILED(hr))
        {
            SAFE_RELEASE(result);
            return hr;
        }
    }

    // Compute the overall bounding box
    if (result->_meshCount > 0)
    {
        result->_boundingBox = result->_meshes[0]->GetAxisAlignedBox();

        for (UINT i = 1; i < result->_meshCount; i++)
        {
            AxisAlignedBox aaBox = result->_meshes[i]->GetAxisAlignedBox();

            Collision::MergeAxisAlignedBoxes(&result->_boundingBox, &result->_boundingBox, &aaBox);
        }
    }

    *output = result;
    return S_OK;
}