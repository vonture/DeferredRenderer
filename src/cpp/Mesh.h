#pragma once

#include "Defines.h"
#include "SDKmesh.h"

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
	DXGI_FORMAT _indexBufferFormat;

	ID3D11Buffer** _vertexBuffers;
	UINT64* _vertexCounts;
	UINT* _vertexStrides;
	UINT* _offsets;
	UINT _vertexBufferCount;
	
	MeshPart* _meshParts;
	UINT _meshPartCount;

	D3D11_INPUT_ELEMENT_DESC* _inputElements;
	UINT _inputElementCount;

	AxisAlignedBox _boundingBox;

public:
	Mesh();
	~Mesh();

	const MeshPart& GetMeshPart(UINT idx) const { return _meshParts[idx]; }
	UINT GetMeshPartCount() const { return _meshPartCount; }

	ID3D11Buffer*const* GetVertexBuffers() const { return _vertexBuffers; }
	const ID3D11Buffer* GetVertexBuffer(UINT idx) const { return _vertexBuffers[idx]; }
	
	const UINT* GetVertexStrides() const { return _vertexStrides; }
	UINT GetVertexStride(UINT idx) const { return _vertexStrides[idx]; }

	const UINT* GetOffsets() const { return _offsets; }
	UINT GetOffset(UINT idx) const { return _offsets[idx]; }

	UINT GetVertexBufferCount() const { return _vertexBufferCount; }

	DXGI_FORMAT GetIndexBufferFormat() const { return _indexBufferFormat; }
	ID3D11Buffer* GetIndexBuffer() const { return _indexBuffer; }

	const D3D11_INPUT_ELEMENT_DESC* GetInputLayout() const { return _inputElements; }
	UINT GetInputElementCount() const { return _inputElementCount; }

	const AxisAlignedBox& GetAxisAlignedBox() const { return _boundingBox; }
	
	HRESULT CreateFromSDKMeshMesh(CDXUTSDKMesh* model, UINT meshIdx);
	void Destroy();
};
