#pragma once

#include "PCH.h"
#include "ContentType.h"
#include "Material.h"
#include "Mesh.h"
#include "xnaCollision.h"

#define INVALID_SAMPLER_SLOT ((UINT)-1)
#define INVALID_BUFFER_SLOT ((UINT)-1)

class Model : public ContentType
{
private:
	UINT _refCount;

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

	const AxisAlignedBox& GetMeshAxisAlignedBox(UINT idx) const { return _meshes[idx].GetAxisAlignedBox(); }
	const AxisAlignedBox& GetAxisAlignedBox() const { return _boundingBox; }

	HRESULT CreateFromFile(ID3D11Device* device, LPCWSTR fileName);
	void Destroy();
	
	HRESULT Render(ID3D11DeviceContext* context, UINT materialBufferSlot = INVALID_BUFFER_SLOT,
		UINT diffuseSlot = INVALID_SAMPLER_SLOT, UINT normalSlot = INVALID_SAMPLER_SLOT, 
		UINT specularSlot = INVALID_SAMPLER_SLOT);
	HRESULT RenderMesh(ID3D11DeviceContext* context, UINT meshIdx, UINT materialBufferSlot = INVALID_BUFFER_SLOT,
		UINT diffuseSlot = INVALID_SAMPLER_SLOT, UINT normalSlot = INVALID_SAMPLER_SLOT, 
		UINT specularSlot = INVALID_SAMPLER_SLOT);
};