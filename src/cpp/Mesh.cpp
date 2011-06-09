#include "Mesh.h"

Mesh::Mesh()
	: _vertexBufferCount(0), _vertexBuffers(NULL), _vertexCounts(NULL), _vertexStrides(NULL), _offsets(NULL), 
	  _indexBuffer(NULL), _meshParts(NULL), _meshPartCount(0), _inputElements(NULL), _inputElementCounts(NULL)
{
}

Mesh::~Mesh()
{
}

HRESULT Mesh::CreateFromSDKMeshMesh(ID3D11Device* device, CDXUTSDKMesh* model, UINT meshIdx)
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
	_inputElements = new D3D11_INPUT_ELEMENT_DESC*[_vertexBufferCount];
	_inputElementCounts = new UINT[_vertexBufferCount];
	for (UINT i = 0; i < _vertexBufferCount; i++)
	{		
		SDKMESH_VERTEX_BUFFER_HEADER* vbHeader = model->GetVBHeader(meshIdx, i);

		// Simply copy the vertex buffers from the original model
		_vertexBuffers[i] = model->GetVB11(meshIdx, i);
		_vertexBuffers[i]->AddRef();
		
		_vertexCounts[i] = model->GetNumVertices(meshIdx, i);
		_vertexStrides[i] = model->GetVertexStride(meshIdx, i);
		_offsets[i] = 0;

		// Count the vertex decl size
		for (_inputElementCounts[i] = 0; vbHeader->Decl[_inputElementCounts[i]].Stream != 0xFF; _inputElementCounts[i]++);

		_inputElements[i] = new D3D11_INPUT_ELEMENT_DESC[_inputElementCounts[i]];
		for (UINT j = 0; j < _inputElementCounts[i]; j++)
		{
			switch (vbHeader->Decl[j].Usage)
			{
				case D3DDECLUSAGE_POSITION:      // 0
					_inputElements[i][j].SemanticName = "POSITION";
					break;
				case D3DDECLUSAGE_BLENDWEIGHT:   // 1
					_inputElements[i][j].SemanticName = "BLENDWEIGHT";
					break;
				case D3DDECLUSAGE_BLENDINDICES:  // 2
					_inputElements[i][j].SemanticName = "BLENDINDICIES";
					break;
				case D3DDECLUSAGE_NORMAL:        // 3
					_inputElements[i][j].SemanticName = "NORMAL";
					break;
				case D3DDECLUSAGE_PSIZE:         // 4
					_inputElements[i][j].SemanticName = "PSIZE";
					break;
				case D3DDECLUSAGE_TEXCOORD:      // 5
					_inputElements[i][j].SemanticName = "TEXCOORD";
					break;
				case D3DDECLUSAGE_TANGENT:       // 6
					_inputElements[i][j].SemanticName = "TANGENT";
					break;
				case D3DDECLUSAGE_BINORMAL:      // 7
					_inputElements[i][j].SemanticName = "BIONORMAL";
					break;
				case D3DDECLUSAGE_TESSFACTOR:    // 8
					_inputElements[i][j].SemanticName = "TESSFACTOR";
					break;
				case D3DDECLUSAGE_POSITIONT:     // 9
					_inputElements[i][j].SemanticName = "POSITIONT";
					break;
				case D3DDECLUSAGE_COLOR:         // 10
					_inputElements[i][j].SemanticName = "COLOR";
					break;
				case D3DDECLUSAGE_FOG:           // 11
					_inputElements[i][j].SemanticName = "FOG";
					break;
				case D3DDECLUSAGE_DEPTH:         // 12
					_inputElements[i][j].SemanticName = "DEPTH";
					break;
				case D3DDECLUSAGE_SAMPLE:        // 13					
					_inputElements[i][j].SemanticName = "SAMPLE";
					break;
			}

			_inputElements[i][j].SemanticIndex = vbHeader->Decl[j].UsageIndex;

			switch (vbHeader->Decl[j].Type)
			{
				case D3DDECLTYPE_FLOAT1:    //  0,   1D float expanded to (value, 0., 0., 1.)
					_inputElements[i][j].Format = DXGI_FORMAT_R32_FLOAT;
					break;
				case D3DDECLTYPE_FLOAT2:    //  1,   2D float expanded to (value, value, 0., 1.)
					_inputElements[i][j].Format = DXGI_FORMAT_R32G32_FLOAT;
					break;
				case D3DDECLTYPE_FLOAT3:    //  2,   3D float expanded to (value, value, value, 1.)
					_inputElements[i][j].Format = DXGI_FORMAT_R32G32B32_FLOAT;
					break;
				case D3DDECLTYPE_FLOAT4:    //  3,   4D float
					_inputElements[i][j].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					break;
				case D3DDECLTYPE_D3DCOLOR:  //  4,   4D packed unsigned bytes mapped to 0. to 1. range
					                        //        Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
					_inputElements[i][j].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					break;
				case D3DDECLTYPE_UBYTE4:    //  5,   4D unsigned byte
					_inputElements[i][j].Format = DXGI_FORMAT_R8G8B8A8_UINT;
					break;
				case D3DDECLTYPE_SHORT2:    //  6,   2D signed short expanded to (value, value, 0., 1.)
					_inputElements[i][j].Format = DXGI_FORMAT_R16G16_SINT;
					break;
				case D3DDECLTYPE_SHORT4:    //  7,   4D signed short
					_inputElements[i][j].Format = DXGI_FORMAT_R16G16B16A16_SINT;
					break;
					
				case D3DDECLTYPE_UBYTE4N:   //  8,   Each of 4 bytes is normalized by dividing to 255.0
					_inputElements[i][j].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					break;
				case D3DDECLTYPE_SHORT2N:   //  9,   2D signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
					_inputElements[i][j].Format = DXGI_FORMAT_R16G16_SNORM;
					break;
				case D3DDECLTYPE_SHORT4N:   // 10,   4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
					_inputElements[i][j].Format = DXGI_FORMAT_R16G16B16A16_SNORM;
					break;
				case D3DDECLTYPE_USHORT2N:  // 11,   2D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,0,1)
					_inputElements[i][j].Format = DXGI_FORMAT_R16G16_UNORM;
					break;
				case D3DDECLTYPE_USHORT4N:  // 12,   4D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,v[2]/65535.0,v[3]/65535.0)
					_inputElements[i][j].Format = DXGI_FORMAT_R16G16B16A16_UNORM;
					break;
				case D3DDECLTYPE_UDEC3:     // 13,   3D unsigned 10 10 10 format expanded to (value, value, value, 1)
					_inputElements[i][j].Format = DXGI_FORMAT_R10G10B10A2_UINT;
					break;
				case D3DDECLTYPE_DEC3N:     // 14,   3D signed 10 10 10 format normalized and expanded to (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1)
					_inputElements[i][j].Format = DXGI_FORMAT_R10G10B10A2_UNORM;
					break;
				case D3DDECLTYPE_FLOAT16_2: // 15,   Two 16-bit floating point values, expanded to (value, value, 0, 1)
					_inputElements[i][j].Format = DXGI_FORMAT_R16G16_FLOAT;
					break;
				case D3DDECLTYPE_FLOAT16_4: // 16,   Four 16-bit floating point values
					_inputElements[i][j].Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
					break;
			}
			
			_inputElements[i][j].InputSlot = 0;			
			_inputElements[i][j].AlignedByteOffset = vbHeader->Decl[j].Offset;
			_inputElements[i][j].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			_inputElements[i][j].InstanceDataStepRate = 0;
		}
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
		SAFE_DELETE(_inputElements[i]);
	}
	SAFE_DELETE_ARRAY(_vertexBuffers);
	SAFE_DELETE_ARRAY(_vertexCounts);
	SAFE_DELETE_ARRAY(_vertexStrides);
	SAFE_DELETE_ARRAY(_offsets);
	SAFE_DELETE_ARRAY(_inputElements);
	SAFE_DELETE_ARRAY(_inputElementCounts);
	_vertexBufferCount = 0;

	SAFE_DELETE_ARRAY(_meshParts);
	_meshPartCount = 0;
}