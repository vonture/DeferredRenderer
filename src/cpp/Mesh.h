#pragma once

#include "Defines.h"
#include "SDKmesh.h"
#include "BoundingObjects.h"

struct MeshPart
{
	UINT64 VertexStart;
    UINT64 VertexCount;
    UINT64 IndexStart;
    UINT64 IndexCount;
    UINT MaterialIndex;
};

class Mesh
{
private:
	ID3D11Buffer* _indexBuffer;

	ID3D11Buffer** _vertexBuffers;
	UINT* _vertexStrides;
	UINT _vertexBufferCount;

	MeshPart* _meshParts;
	UINT _numMeshParts;

	//D3D11_INPUT_ELEMENT_DESC* _inputElements;
	//UINT _inputElementCount;

	BoundingSphere _boundingSphere;
	BoundingBox _boundingBox;

public:
	Mesh();
	~Mesh();

	const MeshPart& GetMeshPart(UINT idx) const { return _meshParts[idx]; }
	UINT GetMeshPartCount() const { return _numMeshParts; }

	ID3D11Buffer*const* GetVertexBuffers() const { return _vertexBuffers; }
	const ID3D11Buffer* GetVertexBuffer(UINT idx) const { return _vertexBuffers[idx]; }
	
	const UINT* GetVertexStrides() const { return _vertexStrides; }
	UINT GetVertexStride(UINT idx) const { return _vertexStrides[idx]; }

	UINT GetVertexBufferCount() const { return _vertexBufferCount; }

	const ID3D11Buffer* GetIndexBuffer() const { return _indexBuffer; }

	const BoundingBox& GetBoundingBox() const { return _boundingBox; }
	const BoundingSphere& GetBoundingSphere() const { return _boundingSphere; }

	HRESULT CreateFromSDKMeshMesh(CDXUTSDKMesh* model, UINT meshIdx);
	void Destroy();
};
