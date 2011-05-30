#include "Model.h"
#include "SDKmesh.h"

Model::Model()
	: _meshes(NULL), _meshCount(0), _materials(NULL), _materialCount(0)
{
}

Model::~Model()
{
}

HRESULT Model::CreateFromSDKMeshFile(ID3D11Device* device, LPCWSTR fileName, bool generateTangents)
{
	HRESULT hr;
	CDXUTSDKMesh sdkMesh;
    sdkMesh.Create(device, fileName);
	
	// Make materials
    _materialCount = sdkMesh.GetNumMaterials();
	_materials = new Material[_materialCount];
    for (UINT i = 0; i < _materialCount; i++)
    {
        V_RETURN(_materials[i].CreateFromSDKMeshMaterial(sdkMesh.GetMaterial(i)));
    }

	// Copy the meshes
	_meshCount = sdkMesh.GetNumMeshes();
	_meshes = new Mesh[_meshCount];
    for (UINT i = 0; i < _meshCount; i++)
	{
		V_RETURN(_meshes[i].CreateFromSDKMeshMesh(&sdkMesh, i));
	}

	sdkMesh.Destroy();

	return S_OK;
}

void Model::Destroy()
{
	for (UINT i = 0; i < _materialCount; i++)
	{
		_materials[i].Destroy();
	}
	SAFE_DELETE_ARRAY(_materials);
	_materialCount = 0;

	for (UINT i = 0; i < _meshCount; i++)
    {
		_meshes[i].Destroy();
	}
	SAFE_DELETE_ARRAY(_meshes);
	_meshCount = 0;
}