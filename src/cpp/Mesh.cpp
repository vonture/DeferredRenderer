#include "Mesh.h"

Mesh::Mesh()
	: _vertexBufferCount(0), _vertexBuffers(NULL), _indexBuffer(NULL), _meshParts(NULL), 
	  _numMeshParts(0)
	  //,_inputElements(NULL), _inputElementCount(0)
{
}

Mesh::~Mesh()
{
}

HRESULT Mesh::CreateFromSDKMeshMesh(CDXUTSDKMesh* model, UINT meshIdx)
{
	SDKMESH_MESH* mesh = model->GetMesh(meshIdx);
	
	// Set the index buffer
	_indexBuffer = model->GetIB11At(mesh->IndexBuffer);
	_indexBuffer->AddRef();
	
	// Set the vertex buffers
	_vertexBufferCount = mesh->NumVertexBuffers;
	_vertexBuffers = new ID3D11Buffer*[_vertexBufferCount];
	_vertexStrides = new UINT[_vertexBufferCount];
	for (UINT i = 0; i < _vertexBufferCount; i++)
	{
		_vertexBuffers[i] = model->GetVB11At(mesh->VertexBuffers[i]);
		_vertexBuffers[i]->AddRef();

		_vertexStrides[i] = model->GetVertexStride(meshIdx, i);
	}
	
	// Copy over the subset information
	_numMeshParts = mesh->NumSubsets;
	_meshParts = new MeshPart[_numMeshParts];
	for (UINT i = 0; i < _numMeshParts; i++)
	{
		SDKMESH_SUBSET* subset = model->GetSubset(meshIdx, i);
		
		_meshParts[i].VertexStart = subset->VertexStart;
		_meshParts[i].VertexCount = subset->VertexCount;
		_meshParts[i].IndexStart = subset->IndexStart;
		_meshParts[i].IndexCount = subset->IndexCount;
		_meshParts[i].MaterialIndex = subset->MaterialID;
	}	

	// Copy over the bounding box/sphere information
	D3DXVECTOR3 meshMid = mesh->BoundingBoxCenter;
	D3DXVECTOR3 meshExtent = mesh->BoundingBoxExtents;

	_boundingSphere.SetPosition(XMVectorSet(meshMid.x, meshMid.y, meshMid.z, 1.0f));
	_boundingSphere.SetRadius(D3DXVec3Length(&meshExtent));

	_boundingBox.SetMin(XMVectorSet(meshMid.x - meshExtent.x, meshMid.y - meshExtent.y, meshMid.z - meshExtent.z, 1.0f));
	_boundingBox.SetMin(XMVectorSet(meshMid.x + meshExtent.x, meshMid.y + meshExtent.y, meshMid.z + meshExtent.z, 1.0f));

	return S_OK;
}

void Mesh::Destroy()
{
	for (UINT i = 0; i < _vertexBufferCount; i++)
	{
		SAFE_RELEASE(_vertexBuffers[i]);
	}
	SAFE_DELETE_ARRAY(_vertexBuffers);
	SAFE_DELETE_ARRAY(_vertexStrides);
	_vertexBufferCount = 0;

	SAFE_RELEASE(_indexBuffer);

	SAFE_DELETE_ARRAY(_meshParts);
	//SAFE_DELETE_ARRAY(_inputElements);
}