#pragma once

#include "Defines.h"
#include "Material.h"
#include "Mesh.h"

class Model
{
private:
	Mesh* _meshes;
	UINT _meshCount;

	Material* _materials;
	UINT _materialCount;

public:
	Model();
	~Model();

	const Mesh& GetMesh(int idx) const { return _meshes[idx]; }
	UINT GetMeshCount() const { return _meshCount; }

	const Material& GetMaterial(int idx) const { return _materials[idx]; }
	UINT GetMaterialCount() const { return _materialCount; }

	HRESULT CreateFromSDKMeshFile(ID3D11Device* device, LPCWSTR fileName, bool generateTangents = false);
	void Destroy();
};