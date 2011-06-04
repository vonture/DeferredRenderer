#pragma once

#include "Defines.h"
#include "Material.h"
#include "Mesh.h"

#define INVALID_SAMPLER_SLOT ((UINT)-1)

class Model
{
private:
	Mesh* _meshes;
	UINT _meshCount;

	Material* _materials;
	UINT _materialCount;

	AxisAlignedBox _boundingBox;

public:
	Model();
	~Model();

	const Mesh& GetMesh(UINT idx) const { return _meshes[idx]; }
	UINT GetMeshCount() const { return _meshCount; }

	const Material& GetMaterial(UINT idx) const { return _materials[idx]; }
	UINT GetMaterialCount() const { return _materialCount; }

	const AxisAlignedBox& GetMeshBoundingBox(UINT idx) const { return _meshes[idx].GetBoundingBox(); }
	const AxisAlignedBox& GetBoundingBox() const { return _boundingBox; }

	HRESULT CreateFromSDKMeshFile(ID3D11Device* device, LPCWSTR fileName);
	void Destroy();
	
	HRESULT Render(ID3D11DeviceContext* context, UINT diffuseSlot = INVALID_SAMPLER_SLOT,
		UINT normalSlot = INVALID_SAMPLER_SLOT, UINT specularSlot = INVALID_SAMPLER_SLOT);
	HRESULT RenderMesh(ID3D11DeviceContext* context, UINT meshIdx, UINT diffuseSlot = INVALID_SAMPLER_SLOT,
		UINT normalSlot = INVALID_SAMPLER_SLOT, UINT specularSlot = INVALID_SAMPLER_SLOT);
};