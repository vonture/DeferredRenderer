#pragma once

#include "PCH.h"
#include "SDKmesh.h"
#include "aiScene.h"
#include "xnaCollision.h"

struct MeshPart
{
	UINT VertexStart;
    UINT VertexCount;
    UINT IndexStart;
    UINT IndexCount;
    UINT MaterialIndex;
};

class Mesh
{
private:
	ID3D11Buffer* _indexBuffer;
	UINT _indexCount;
	DXGI_FORMAT _indexBufferFormat;

	ID3D11Buffer* _vertexBuffer;
	UINT _vertexCount;
	UINT _vertexStride;
	
	MeshPart* _meshParts;
	UINT _meshPartCount;

	D3D11_INPUT_ELEMENT_DESC* _inputElements;
	UINT _inputElementCount;

	AxisAlignedBox _boundingBox;

	void CreateInputElements(D3DVERTEXELEMENT9* declaration);

public:
	Mesh();
	~Mesh();

	const MeshPart& GetMeshPart(UINT idx) const { return _meshParts[idx]; }
	UINT GetMeshPartCount() const { return _meshPartCount; }

	ID3D11Buffer* GetVertexBuffer() const { return _vertexBuffer; }

	const UINT GetVertexStride() const { return _vertexStride; }
	
	DXGI_FORMAT GetIndexBufferFormat() const { return _indexBufferFormat; }
	ID3D11Buffer* GetIndexBuffer() const { return _indexBuffer; }

	const D3D11_INPUT_ELEMENT_DESC* GetInputLayout() const { return _inputElements; }
	UINT GetInputElementCount() const { return _inputElementCount; }

	const AxisAlignedBox& GetAxisAlignedBox() const { return _boundingBox; }
	
	HRESULT CreateFromSDKMeshMesh(ID3D11Device* device, IDirect3DDevice9* d3d9Device, 
		const WCHAR* modelPath, SDKMesh* model,	UINT meshIdx);

	HRESULT CreateFromASSIMPMesh(ID3D11Device* device, const aiScene* scene, UINT meshIdx);

	void Destroy();
};
