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

	BoundingBox _boundingBox;
	BoundingSphere _boundingSphere;

public:
	Model();
	~Model();

	const Mesh& GetMesh(UINT idx) const { return _meshes[idx]; }
	UINT GetMeshCount() const { return _meshCount; }

	const Material& GetMaterial(UINT idx) const { return _materials[idx]; }
	UINT GetMaterialCount() const { return _materialCount; }

	const BoundingBox& GetBoundingBox(UINT idx) const { return _meshes[idx].GetBoundingBox(); }
	const BoundingSphere& GetBoundingSphere(UINT idx) const { return _meshes[idx].GetBoundingSphere(); }

	const BoundingBox& GetBoundingBox() const { return _boundingBox; }
	const BoundingSphere& GetBoundingSphere() const { return _boundingSphere; }

	HRESULT CreateFromSDKMeshFile(ID3D11Device* device, LPCWSTR fileName);
	void Destroy();
	
	HRESULT Render(ID3D11DeviceContext* context, UINT diffuseSlot = INVALID_SAMPLER_SLOT,
		UINT normalSlot = INVALID_SAMPLER_SLOT, UINT specularSlot = INVALID_SAMPLER_SLOT);
	HRESULT RenderPart(ID3D11DeviceContext* context, UINT partIdx, UINT diffuseSlot = INVALID_SAMPLER_SLOT,
		UINT normalSlot = INVALID_SAMPLER_SLOT, UINT specularSlot = INVALID_SAMPLER_SLOT);
};