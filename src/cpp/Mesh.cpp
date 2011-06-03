#include "Mesh.h"

Mesh::Mesh()
	: _vertexBufferCount(0), _vertexBuffers(NULL), _vertexCounts(NULL), _vertexStrides(NULL), _offsets(NULL), 
	  _indexBuffer(NULL), _meshParts(NULL), _meshPartCount(0), _inputElements(NULL), _inputElementCount(0)
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
	
	_indexBufferFormat = (model->GetIndexType(meshIdx) == IT_32BIT) ? 
		DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	
	// Set the vertex buffers
	_vertexBufferCount = mesh->NumVertexBuffers;	
	_vertexBuffers = new ID3D11Buffer*[_vertexBufferCount];
	_vertexCounts = new UINT64[_vertexBufferCount];
	_vertexStrides = new UINT[_vertexBufferCount];
	_offsets = new UINT[_vertexBufferCount];
	for (UINT i = 0; i < _vertexBufferCount; i++)
	{		
		// Simply copy the vertex buffers from the original model
		_vertexBuffers[i] = model->GetVB11(meshIdx, i);
		_vertexBuffers[i]->AddRef();
		
		_vertexCounts[i] = model->GetNumVertices(meshIdx, i);
		_vertexStrides[i] = model->GetVertexStride(meshIdx, i);
		_offsets[i] = 0;

		// Create the input layout
		const D3D11_INPUT_ELEMENT_DESC layout_mesh[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		_inputElementCount = 5;
		_inputElements = new D3D11_INPUT_ELEMENT_DESC[_inputElementCount];
		memcpy(_inputElements, layout_mesh, sizeof(D3D11_INPUT_ELEMENT_DESC) * _inputElementCount);		
	}

	// Copy over the subset information
	_meshPartCount = mesh->NumSubsets;
	_meshParts = new MeshPart[_meshPartCount];
	for (UINT i = 0; i < _meshPartCount; i++)
	{
		SDKMESH_SUBSET* subset = model->GetSubset(meshIdx, i);
		
		_meshParts[i].VertexStart = subset->VertexStart;
		_meshParts[i].VertexCount = subset->VertexCount;
		_meshParts[i].IndexStart = subset->IndexStart;
		_meshParts[i].IndexCount = subset->IndexCount;
		_meshParts[i].MaterialIndex = subset->MaterialID;
	}	

	// Copy over the bounding box information
	D3DXVECTOR3 meshMid = mesh->BoundingBoxCenter;
	D3DXVECTOR3 meshExtent = mesh->BoundingBoxExtents;

	_boundingBox.Center = XMFLOAT3(meshMid.x, meshMid.y, meshMid.z);
	_boundingBox.Extents = XMFLOAT3(meshExtent.x, meshExtent.y, meshExtent.z);
	
	return S_OK;
}

void Mesh::Destroy()
{
	SAFE_RELEASE(_indexBuffer);

	for (UINT i = 0; i < _vertexBufferCount; i++)
	{
		SAFE_RELEASE(_vertexBuffers[i]);
	}
	SAFE_DELETE_ARRAY(_vertexBuffers);
	SAFE_DELETE_ARRAY(_vertexCounts);
	SAFE_DELETE_ARRAY(_vertexStrides);
	SAFE_DELETE_ARRAY(_offsets);
	_vertexBufferCount = 0;

	SAFE_DELETE_ARRAY(_meshParts);
	_meshPartCount = 0;

	SAFE_DELETE_ARRAY(_inputElements);
	_inputElementCount = 0;
}