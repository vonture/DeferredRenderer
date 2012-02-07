#include "PCH.h"
#include "Mesh.h"
#include "Logger.h"

Mesh::Mesh()
	: _indexBuffer(NULL), _indexCount(0), _vertexBuffer(NULL), _vertexCount(0), _vertexStride(0), 
	  _meshParts(NULL), _meshPartCount(0), _inputElements(NULL), _alphaCutoutEnabled(true),
	  _drawBackFaces(false), _name(NULL)
{
}

Mesh::~Mesh()
{
	Destroy();
}

D3DXVECTOR3 Mesh::Perpendicular(const D3DXVECTOR3& vec)
{
    _ASSERT(D3DXVec3Length(&vec) >= EPSILON);

    D3DXVECTOR3 perp;

    float x = fabs(vec.x);
    float y = fabs(vec.y);
    float z = fabs(vec.z);
    float minVal = min(x, y);
    minVal = min(minVal, z);

    if(minVal == x)
	{
		D3DXVECTOR3 unitX = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
        D3DXVec3Cross(&perp, &vec, &unitX);
	}
    else if(minVal == y)
	{
		D3DXVECTOR3 unitY = D3DXVECTOR3(0.0f, 1.0f, 0.0f); 
        D3DXVec3Cross(&perp, &vec, &unitY);
	}
    else
	{
		D3DXVECTOR3 unitZ = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
        D3DXVec3Cross(&perp, &vec, &unitZ);
	}

    D3DXVec3Normalize(&perp, &perp);
    return perp;
}

ID3DXMesh* Mesh::GenerateTangentFrame(ID3DXMesh* mesh, UINT numVertices, UINT numIndices,
	DXGI_FORMAT indexType, IDirect3DDevice9* d3d9Device)
{
	HRESULT hr;

    // make sure we have a texture coordinate
    D3DVERTEXELEMENT9 decl[MAX_FVF_DECL_SIZE];
    V(mesh->GetDeclaration(decl));
    bool foundTexCoord = false;
    bool foundNormal = false;
    for (UINT i = 0; i < MAX_FVF_DECL_SIZE; ++i)
    {
        if (decl[i].Stream == 0xFF)
            break;
        else if(decl[i].Usage == D3DDECLUSAGE_TEXCOORD && decl[i].UsageIndex == 0)
            foundTexCoord = true;
        else if(decl[i].Usage == D3DDECLUSAGE_NORMAL && decl[i].UsageIndex == 0)
            foundNormal = true;
    }

    if (!foundTexCoord || !foundNormal)
	{
		_ASSERT(false);
	}

    // Clone the mesh with a new declaration
    D3DVERTEXELEMENT9 newDecl[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
        { 0, 24, D3DDECLTYPE_FLOAT2,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 32, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,  0 },
        { 0, 44, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0 },
        D3DDECL_END()
    };

    ID3DXMesh* clonedMesh = NULL;

    UINT options = D3DXMESH_MANAGED;
    if (indexType == DXGI_FORMAT_R32_UINT)
	{
        options |= D3DXMESH_32BIT;
	}
    V(mesh->CloneMesh(options, newDecl, d3d9Device, &clonedMesh));

    // Compute the tangent frame for each vertex. The following code is based on
    // "Computing Tangent Space Basis Vectors for an Arbitrary Mesh", by Eric Lengyel
    // http://www.terathon.com/code/tangent.html

    // Make temporary arrays for the tangent and the bitangent
    D3DXVECTOR3* tangents = new D3DXVECTOR3[numVertices];
    D3DXVECTOR3* bitangents = new D3DXVECTOR3[numVertices];
    ZeroMemory(tangents, numVertices * sizeof(D3DXVECTOR3));
    ZeroMemory(bitangents, numVertices * sizeof(D3DXVECTOR3));

    // Get the vertex and index buffer
    struct Vertex
    {
        D3DXVECTOR3 Position;
        D3DXVECTOR3 Normal;
        D3DXVECTOR2 TexCoord;
        D3DXVECTOR3 Tangent;
        D3DXVECTOR3 Binormal;
    };

    void* indices = NULL;
    Vertex* vertices = NULL;
    V(clonedMesh->LockIndexBuffer(D3DLOCK_READONLY, &indices));
    V(clonedMesh->LockVertexBuffer(D3DLOCK_READONLY, reinterpret_cast<void**>(&vertices)));

    UINT* indices32 = reinterpret_cast<UINT*>(indices);
    WORD* indices16 = reinterpret_cast<WORD*>(indices);

    // Loop through each triangle
	for (UINT i = 0; i < numIndices; i += 3)
    {
        UINT i1, i2, i3;
        if (indexType == DXGI_FORMAT_R32_UINT)
        {
            i1 = indices32[i + 0];
            i2 = indices32[i + 1];
            i3 = indices32[i + 2];
        }
        else
        {
            i1 = indices16[i + 0];
            i2 = indices16[i + 1];
            i3 = indices16[i + 2];
        }

        const D3DXVECTOR3& v1 = vertices[i1].Position;
        const D3DXVECTOR3& v2 = vertices[i2].Position;
        const D3DXVECTOR3& v3 = vertices[i3].Position;

        const D3DXVECTOR2& w1 = vertices[i1].TexCoord;
        const D3DXVECTOR2& w2 = vertices[i2].TexCoord;
        const D3DXVECTOR2& w3 = vertices[i3].TexCoord;

        float x1 = v2.x - v1.x;
        float x2 = v3.x - v1.x;
        float y1 = v2.y - v1.y;
        float y2 = v3.y - v1.y;
        float z1 = v2.z - v1.z;
        float z2 = v3.z - v1.z;

        float s1 = w2.x - w1.x;
        float s2 = w3.x - w1.x;
        float t1 = w2.y - w1.y;
        float t2 = w3.y - w1.y;

        float r = 1.0f / (s1 * t2 - s2 * t1);
        D3DXVECTOR3 sDir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
        D3DXVECTOR3 tDir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

        tangents[i1] += sDir;
        tangents[i2] += sDir;
        tangents[i3] += sDir;

        bitangents[i1] += tDir;
        bitangents[i2] += tDir;
        bitangents[i3] += tDir;
    }
	
    for (UINT i = 0; i < numVertices; ++i)
    {
        D3DXVECTOR3& n = vertices[i].Normal;

        D3DXVECTOR3& t = tangents[i];

        // Gram-Schmidt orthogonalize
        D3DXVECTOR3 tangent = (t - n * D3DXVec3Dot(&n, &t));
        bool zeroTangent = false;
        if(D3DXVec3Length(&tangent) > EPSILON)
		{
            D3DXVec3Normalize(&tangent, &tangent);
		}
        else if(D3DXVec3Length(&n) > EPSILON)
        {
            tangent = Perpendicular(n);
            zeroTangent = true;
        }

        float sign = 1.0f;

        if(!zeroTangent)
        {
            D3DXVECTOR3 b;
            D3DXVec3Cross(&b, &n, &t);
            sign = (D3DXVec3Dot(&b, &bitangents[i]) < 0.0f) ? -1.0f : 1.0f;
        }

        // Store the tangent + binormal
        vertices[i].Tangent = tangent;

        D3DXVec3Cross(&vertices[i].Binormal, &n, &tangent);
        vertices[i].Binormal *= sign;
    }

    // Clean up our temporary arrays
    delete[] tangents;
    delete[] bitangents;

    V(clonedMesh->UnlockVertexBuffer());
    V(clonedMesh->UnlockIndexBuffer());

    mesh->Release();

    return clonedMesh;
}

void Mesh::CreateInputElements(D3DVERTEXELEMENT9* declaration, D3D11_INPUT_ELEMENT_DESC** output, 
	UINT* elementCount)
{
    std::map<BYTE, LPCSTR> nameMap;
    nameMap[D3DDECLUSAGE_POSITION] = "POSITION";
    nameMap[D3DDECLUSAGE_BLENDWEIGHT] = "BLENDWEIGHT";
    nameMap[D3DDECLUSAGE_BLENDINDICES] = "BLENDINDICES";
    nameMap[D3DDECLUSAGE_NORMAL] = "NORMAL";
    nameMap[D3DDECLUSAGE_TEXCOORD] = "TEXCOORD";
    nameMap[D3DDECLUSAGE_TANGENT] = "TANGENT";
    nameMap[D3DDECLUSAGE_BINORMAL] = "BINORMAL";
    nameMap[D3DDECLUSAGE_COLOR] = "COLOR";

    std::map<BYTE, DXGI_FORMAT> formatMap;
    formatMap[D3DDECLTYPE_FLOAT1] = DXGI_FORMAT_R32_FLOAT;
    formatMap[D3DDECLTYPE_FLOAT2] = DXGI_FORMAT_R32G32_FLOAT;
    formatMap[D3DDECLTYPE_FLOAT3] = DXGI_FORMAT_R32G32B32_FLOAT;
    formatMap[D3DDECLTYPE_FLOAT4] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    formatMap[D3DDECLTYPE_D3DCOLOR] = DXGI_FORMAT_R8G8B8A8_UNORM;
    formatMap[D3DDECLTYPE_UBYTE4] = DXGI_FORMAT_R8G8B8A8_UINT;
    formatMap[D3DDECLTYPE_SHORT2] = DXGI_FORMAT_R16G16_SINT;
    formatMap[D3DDECLTYPE_SHORT4] = DXGI_FORMAT_R16G16B16A16_SINT;
    formatMap[D3DDECLTYPE_UBYTE4N] = DXGI_FORMAT_R8G8B8A8_UNORM;
    formatMap[D3DDECLTYPE_SHORT2N] = DXGI_FORMAT_R16G16_SNORM;
    formatMap[D3DDECLTYPE_SHORT4N] = DXGI_FORMAT_R16G16B16A16_SNORM;
    formatMap[D3DDECLTYPE_USHORT2N] = DXGI_FORMAT_R16G16_UNORM;
    formatMap[D3DDECLTYPE_USHORT4N] = DXGI_FORMAT_R16G16B16A16_UNORM;
    formatMap[D3DDECLTYPE_UDEC3] = DXGI_FORMAT_R10G10B10A2_UINT;
    formatMap[D3DDECLTYPE_DEC3N] = DXGI_FORMAT_R10G10B10A2_UNORM;
    formatMap[D3DDECLTYPE_FLOAT16_2] = DXGI_FORMAT_R16G16_FLOAT;
    formatMap[D3DDECLTYPE_FLOAT16_4] = DXGI_FORMAT_R16G16B16A16_FLOAT;

    // Figure out the number of elements
	for (*elementCount = 0; declaration[*elementCount].Stream != 0xFF; (*elementCount)++);

	*output = new D3D11_INPUT_ELEMENT_DESC[*elementCount];
    for (UINT i = 0; i < (*elementCount); i++)
    {
        (*output)[i].InputSlot = 0;
        (*output)[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        (*output)[i].InstanceDataStepRate = 0;
        (*output)[i].SemanticName = nameMap[declaration[i].Usage];
        (*output)[i].Format = formatMap[declaration[i].Type];
        (*output)[i].AlignedByteOffset = declaration[i].Offset;
        (*output)[i].SemanticIndex = declaration[i].UsageIndex;
    }
}

HRESULT Mesh::CreateFromSDKMeshMesh(ID3D11Device* device, IDirect3DDevice9* d3d9Device, 
	const WCHAR* modelPath, SDKMesh* model, UINT meshIdx)
{
	HRESULT hr;
	
	// Figure out the index type
    UINT ops = D3DXMESH_MANAGED;
    UINT indexSize = 2;
    _indexBufferFormat = DXGI_FORMAT_R16_UINT;
    if (model->GetIndexType(meshIdx) == IT_32BIT)
    {
        ops |= D3DXMESH_32BIT;
        indexSize = 4;
        _indexBufferFormat = DXGI_FORMAT_R32_UINT;
    }

	// Make a D3DX mesh
    ID3DXMesh* d3dxMesh = NULL;
	SDKMESH_MESH* meshMesh = model->GetMesh(meshIdx);
    UINT numPrims = static_cast<UINT>(model->GetNumIndices(meshIdx) / 3);
    UINT numVerts = static_cast<UINT>(model->GetNumVertices(meshIdx, 0));
    UINT vbIndex = meshMesh->VertexBuffers[0];
    UINT ibIndex = meshMesh->IndexBuffer;
    const D3DVERTEXELEMENT9* vbElements = model->VBElements(vbIndex);
	
    V_RETURN(D3DXCreateMesh(numPrims, numVerts, ops, vbElements, d3d9Device, &d3dxMesh));

    // Copy in vertex data
    BYTE* verts = NULL;
    BYTE* srcVerts = reinterpret_cast<BYTE*>(model->GetRawVerticesAt(vbIndex));
    UINT vbStride = model->GetVertexStride(meshIdx, 0);
    UINT declStride = D3DXGetDeclVertexSize(vbElements, 0);
    V_RETURN(d3dxMesh->LockVertexBuffer(0, reinterpret_cast<void**>(&verts)));
    for (UINT vertIdx = 0; vertIdx < numVerts; ++vertIdx)
    {
        memcpy(verts, srcVerts, declStride);
        verts += declStride;
        srcVerts += vbStride;
    }
    V_RETURN(d3dxMesh->UnlockVertexBuffer());

    // Copy in index data
    void* indices = NULL;
    void* srcIndices = model->GetRawIndicesAt(ibIndex);
    V_RETURN(d3dxMesh->LockIndexBuffer(0, &indices));
    memcpy(indices, srcIndices, numPrims * 3 * indexSize);
    V_RETURN(d3dxMesh->UnlockIndexBuffer());

    // Set up the attribute table
    DWORD* attributeBuffer = NULL;
    V_RETURN(d3dxMesh->LockAttributeBuffer(0, &attributeBuffer));

    UINT numSubsets = model->GetNumSubsets(meshIdx);
    D3DXATTRIBUTERANGE* attributes = new D3DXATTRIBUTERANGE[numSubsets];
    for (UINT i = 0; i < numSubsets; ++i)
    {
        SDKMESH_SUBSET* subset = model->GetSubset(meshIdx, i);
        attributes[i].AttribId = subset->MaterialID;
        attributes[i].FaceStart = static_cast<DWORD>(subset->IndexStart / 3);
        attributes[i].FaceCount = static_cast<DWORD>(subset->IndexCount / 3);
        attributes[i].VertexStart = static_cast<DWORD>(subset->VertexStart);
        attributes[i].VertexCount = numVerts;

        for (UINT faceIdx = attributes[i].FaceStart; faceIdx < attributes[i].FaceStart + attributes[i].FaceCount; ++faceIdx)
		{
            attributeBuffer[faceIdx] = subset->MaterialID;
		}
    }

    V_RETURN(d3dxMesh->UnlockAttributeBuffer());

    d3dxMesh->SetAttributeTable(attributes, numSubsets);
	
	_vertexStride = d3dxMesh->GetNumBytesPerVertex();
    _vertexCount = d3dxMesh->GetNumVertices();
    _indexCount = d3dxMesh->GetNumFaces() * 3;

	d3dxMesh = GenerateTangentFrame(d3dxMesh, _vertexCount, _indexCount, _indexBufferFormat, d3d9Device);

	_vertexStride = d3dxMesh->GetNumBytesPerVertex();
    _vertexCount = d3dxMesh->GetNumVertices();
    _indexCount = d3dxMesh->GetNumFaces() * 3;

	// Convert the D3D9 vertex declaration to a D3D11 input element desc
    D3DVERTEXELEMENT9 declaration[MAX_FVF_DECL_SIZE];
    V_RETURN(d3dxMesh->GetDeclaration(declaration));
    CreateInputElements(declaration, &_inputElements, &_inputElementCount);

	// Copy over the vertex data
    void* vertices = NULL;
    V_RETURN(d3dxMesh->LockVertexBuffer(0, &vertices));

    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.ByteWidth = _vertexStride * _vertexCount;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = vertices;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;
    V_RETURN(device->CreateBuffer(&bufferDesc, &initData, &_vertexBuffer));

	 // Compute bounding box
    D3DXVECTOR3 boxMin, boxMax;
    V_RETURN(D3DXComputeBoundingBox(reinterpret_cast<D3DXVECTOR3*>(vertices), _vertexCount,
		_vertexStride, &boxMin, &boxMax));

	_boundingBox.Center = (XMFLOAT3)((boxMax + boxMin) * 0.5f);
	_boundingBox.Extents = (XMFLOAT3)((boxMax - boxMin) * 0.5f);

    V_RETURN(d3dxMesh->UnlockVertexBuffer());

	// Copy over the index data
    void* finalIndices = NULL;
    V_RETURN(d3dxMesh->LockIndexBuffer(0, &finalIndices));

    UINT finalIndexSize = _indexBufferFormat == DXGI_FORMAT_R32_UINT ? 4 : 2;
    bufferDesc.ByteWidth = finalIndexSize * _indexCount;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    initData.pSysMem = finalIndices;
    V_RETURN(device->CreateBuffer(&bufferDesc, &initData, &_indexBuffer));

    V_RETURN(d3dxMesh->UnlockIndexBuffer());

	// Copy the name
	UINT nameLen = strlen(model->GetMesh(meshIdx)->Name);
	if (nameLen > 0)
	{
		_name = new WCHAR[nameLen + 1];
		AnsiToWString(model->GetMesh(meshIdx)->Name, _name, nameLen + 1);
	}
	else
	{
		_name = new WCHAR[MAX_PATH];
		swprintf_s(_name, MAX_PATH, L"Mesh %u", meshIdx);
	}

	// Copy in the subset info
    DWORD subsetCount = 0;
    V_RETURN(d3dxMesh->GetAttributeTable(NULL, &subsetCount));
    D3DXATTRIBUTERANGE* attributeTable = new D3DXATTRIBUTERANGE[subsetCount];
    V_RETURN(d3dxMesh->GetAttributeTable(attributeTable, &subsetCount));

	_meshPartCount = subsetCount;
	_meshParts = new MeshPart[_meshPartCount];
    for(UINT i = 0; i < numSubsets; ++i)
    {
        _meshParts[i].VertexStart = attributeTable[i].VertexStart;
        _meshParts[i].IndexStart = attributeTable[i].FaceStart * 3;
        _meshParts[i].IndexCount = attributeTable[i].FaceCount * 3;
        _meshParts[i].MaterialIndex = attributeTable[i].AttribId;
    }
	
	delete[] attributes;
	delete[] attributeTable;
	SAFE_RELEASE(d3dxMesh);
	
	return S_OK;
}

HRESULT Mesh::CreateFromASSIMPMesh(ID3D11Device* device, const aiScene* scene, UINT meshIdx)
{
	HRESULT hr;

	aiMesh* mesh = scene->mMeshes[meshIdx];

	if (!mesh->HasFaces() || !mesh->HasNormals() || !mesh->HasPositions() || 
		!mesh->HasTangentsAndBitangents() || mesh->GetNumUVChannels() == 0)
	{
		return E_FAIL;
	}

	// Create an array of verticies
	_vertexStride = sizeof(Vertex);
	
	UINT uvChannel = 0;

	_vertexCount = mesh->mNumVertices;
	Vertex* verts = new Vertex[_vertexCount];
	for (UINT i = 0; i < _vertexCount; i++)
	{
		verts[i].Position = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		verts[i].TexCoord = XMFLOAT2(mesh->mTextureCoords[uvChannel][i].x, mesh->mTextureCoords[uvChannel][i].y);
		verts[i].Normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);		
		verts[i].Tangent = XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
		verts[i].Bitangent = XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
	}

	// Compute the bounding box for the vertices
	Collision::ComputeBoundingAxisAlignedBoxFromPoints(&_boundingBox, _vertexCount, (XMFLOAT3*)verts, 
		sizeof(Vertex));
	
	// Build the vertex buffer
	D3D11_BUFFER_DESC vbDesc =
    {
        _vertexCount * sizeof(Vertex),
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_VERTEX_BUFFER,
        0,
        0
    };

	D3D11_SUBRESOURCE_DATA vbInitData;
    vbInitData.pSysMem = verts;
    vbInitData.SysMemPitch = 0;
    vbInitData.SysMemSlicePitch = 0;

	V_RETURN(device->CreateBuffer(&vbDesc, &vbInitData, &_vertexBuffer));

	// Done with the verts, free them
	SAFE_DELETE_ARRAY(verts);

	// Create the indicies
	_indexBufferFormat = DXGI_FORMAT_R32_UINT;
	_indexCount = mesh->mNumFaces * 3;	
	UINT* indices = new UINT[_indexCount];
	for (UINT i = 0; i < _indexCount / 3; i++)
	{
		for (UINT j = 0; j < 3; j++)
		{
			indices[i * 3 + j] = mesh->mFaces[i].mIndices[j];
		}
	}
	_meshPartCount = 1;
	_meshParts = new MeshPart[_meshPartCount];
	
	// Create the index buffer
	D3D11_BUFFER_DESC ibDesc =
    {
        _indexCount * sizeof(UINT),
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_INDEX_BUFFER,
        0,
        0
    };

    D3D11_SUBRESOURCE_DATA ibInitData;
    ibInitData.pSysMem = indices;
    ibInitData.SysMemPitch = 0;
    ibInitData.SysMemSlicePitch = 0;

	V_RETURN(device->CreateBuffer(&ibDesc, &ibInitData, &_indexBuffer));

	// delete the indices array
	SAFE_DELETE_ARRAY(indices);
	
	// Create the mesh parts, only one per mesh
	_meshParts[0].IndexStart = 0;
	_meshParts[0].IndexCount = _indexCount;
	_meshParts[0].MaterialIndex = mesh->mMaterialIndex;
	_meshParts[0].VertexStart = 0;

	// Copy the name
	if (mesh->mName.length > 0)
	{
		_name = new WCHAR[mesh->mName.length + 1];
		AnsiToWString(mesh->mName.data, _name, mesh->mName.length + 1);
	}
	else
	{
		_name = new WCHAR[MAX_PATH];
		swprintf_s(_name, MAX_PATH, L"Mesh %u", meshIdx);
	}

	// Create the input elements
	_inputElementCount = 5;
	const D3D11_INPUT_ELEMENT_DESC layout_mesh[] =
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	
	_inputElements = new D3D11_INPUT_ELEMENT_DESC[_inputElementCount];
	memcpy(_inputElements, layout_mesh, sizeof(D3D11_INPUT_ELEMENT_DESC) * _inputElementCount);

	return S_OK;
}

HRESULT Mesh::CompileFromASSIMPMesh(ID3D11Device* device, const aiScene* scene, UINT meshIdx, std::ostream* output )
{
	aiMesh* mesh = scene->mMeshes[meshIdx];

	if (!mesh->HasFaces() || !mesh->HasNormals() || !mesh->HasPositions() || 
		!mesh->HasTangentsAndBitangents() || mesh->GetNumUVChannels() == 0)
	{
		return E_FAIL;
	}


	// Copy the name
	if (mesh->mName.length > 0)
	{
		WCHAR* name = new WCHAR[mesh->mName.length + 1];
		AnsiToWString(mesh->mName.data, name, mesh->mName.length + 1);

		if (!output->write((const char*)&(mesh->mName.length), sizeof(UINT)))
		{
			delete[] name;
			return E_FAIL;
		}

		if (!output->write((const char*)name, sizeof(WCHAR) * mesh->mName.length))
		{
			delete[] name;
			return E_FAIL;
		}

		delete[] name;
	}
	else
	{
		WCHAR* name = new WCHAR[MAX_PATH];
		swprintf_s(name, MAX_PATH, L"Mesh %u", meshIdx);

		UINT len = wcslen(name);

		if (!output->write((const char*)&len, sizeof(UINT)))
		{
			delete[] name;
			return E_FAIL;
		}

		if (!output->write((const char*)name, sizeof(WCHAR) * len))
		{
			delete[] name;
			return E_FAIL;
		}

		delete[] name;
	}

	UINT uvChannel = 0;

	UINT vertexCount = mesh->mNumVertices;
	if (!output->write((const char*)&vertexCount, sizeof(UINT)))
	{
		return E_FAIL;
	}

	Vertex vert;
	for (UINT i = 0; i < vertexCount; i++)
	{
		vert.Position = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vert.TexCoord = XMFLOAT2(mesh->mTextureCoords[uvChannel][i].x, mesh->mTextureCoords[uvChannel][i].y);
		vert.Normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);		
		vert.Tangent = XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
		vert.Bitangent = XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);

		if (!output->write((const char*)&vert, sizeof(Vertex)))
		{
			return E_FAIL;
		}
	}
	
	// Create the indices
	DXGI_FORMAT format = DXGI_FORMAT_R32_UINT;
	if (!output->write((const char*)&format, sizeof(DXGI_FORMAT)))
	{
		return E_FAIL;
	}

	UINT indexCount = mesh->mNumFaces * 3;
	if (!output->write((const char*)&indexCount, sizeof(UINT)))
	{
		return E_FAIL;
	}

	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		if (!output->write((const char*)mesh->mFaces[i].mIndices, sizeof(UINT) * 3))
		{
			return E_FAIL;
		}		
	}

	MeshPart part;
	part.IndexStart = 0;
	part.IndexCount = indexCount;
	part.MaterialIndex = mesh->mMaterialIndex;
	part.VertexStart = 0;

	UINT partCount = 1;

	if (!output->write((const char*)&partCount, sizeof(UINT)))
	{
		return E_FAIL;
	}

	if (!output->write((const char*)&part, sizeof(MeshPart)))
	{
		return E_FAIL;
	}

	return S_OK;
}

void Mesh::Destroy()
{
	SAFE_RELEASE(_indexBuffer);
	_indexCount = 0;

	SAFE_RELEASE(_vertexBuffer);
	_vertexCount = 0;
	_vertexStride = 0;

	SAFE_DELETE_ARRAY(_meshParts);
	_meshPartCount = 0;

	SAFE_DELETE_ARRAY(_inputElements);
	_inputElementCount = 0;

	SAFE_DELETE(_name);
}

HRESULT Mesh::CompileFromSDKMeshMesh(ID3D11Device* device, IDirect3DDevice9* d3d9Device, 
	const WCHAR* modelPath, SDKMesh* model,	UINT meshIdx, std::ostream* output)
{
	HRESULT hr;

	// Figure out the index type
	UINT ops = D3DXMESH_MANAGED;
	UINT indexSize = 2;
	DXGI_FORMAT indexBufferFormat = DXGI_FORMAT_R16_UINT;
	if (model->GetIndexType(meshIdx) == IT_32BIT)
	{
		ops |= D3DXMESH_32BIT;
		indexSize = 4;
		indexBufferFormat = DXGI_FORMAT_R32_UINT;
	}

	// Make a D3DX mesh
	ID3DXMesh* d3dxMesh = NULL;
	SDKMESH_MESH* meshMesh = model->GetMesh(meshIdx);
	UINT numPrims = static_cast<UINT>(model->GetNumIndices(meshIdx) / 3);
	UINT numVerts = static_cast<UINT>(model->GetNumVertices(meshIdx, 0));
	UINT vbIndex = meshMesh->VertexBuffers[0];
	UINT ibIndex = meshMesh->IndexBuffer;
	const D3DVERTEXELEMENT9* vbElements = model->VBElements(vbIndex);

	V_RETURN(D3DXCreateMesh(numPrims, numVerts, ops, vbElements, d3d9Device, &d3dxMesh));

	// Copy in vertex data
	BYTE* verts = NULL;
	BYTE* srcVerts = reinterpret_cast<BYTE*>(model->GetRawVerticesAt(vbIndex));
	UINT vbStride = model->GetVertexStride(meshIdx, 0);
	UINT declStride = D3DXGetDeclVertexSize(vbElements, 0);
	V_RETURN(d3dxMesh->LockVertexBuffer(0, reinterpret_cast<void**>(&verts)));
	for (UINT vertIdx = 0; vertIdx < numVerts; ++vertIdx)
	{
		memcpy(verts, srcVerts, declStride);
		verts += declStride;
		srcVerts += vbStride;
	}
	V_RETURN(d3dxMesh->UnlockVertexBuffer());

	// Copy in index data
	void* indices = NULL;
	void* srcIndices = model->GetRawIndicesAt(ibIndex);
	V_RETURN(d3dxMesh->LockIndexBuffer(0, &indices));
	memcpy(indices, srcIndices, numPrims * 3 * indexSize);
	V_RETURN(d3dxMesh->UnlockIndexBuffer());

	// Set up the attribute table
	DWORD* attributeBuffer = NULL;
	V_RETURN(d3dxMesh->LockAttributeBuffer(0, &attributeBuffer));

	UINT numSubsets = model->GetNumSubsets(meshIdx);
	D3DXATTRIBUTERANGE* attributes = new D3DXATTRIBUTERANGE[numSubsets];
	for (UINT i = 0; i < numSubsets; ++i)
	{
		SDKMESH_SUBSET* subset = model->GetSubset(meshIdx, i);
		attributes[i].AttribId = subset->MaterialID;
		attributes[i].FaceStart = static_cast<DWORD>(subset->IndexStart / 3);
		attributes[i].FaceCount = static_cast<DWORD>(subset->IndexCount / 3);
		attributes[i].VertexStart = static_cast<DWORD>(subset->VertexStart);
		attributes[i].VertexCount = numVerts;

		for (UINT faceIdx = attributes[i].FaceStart; faceIdx < attributes[i].FaceStart + attributes[i].FaceCount; ++faceIdx)
		{
			attributeBuffer[faceIdx] = subset->MaterialID;
		}
	}

	V_RETURN(d3dxMesh->UnlockAttributeBuffer());

	d3dxMesh->SetAttributeTable(attributes, numSubsets);

	UINT vertexStride = d3dxMesh->GetNumBytesPerVertex();
	UINT vertexCount = d3dxMesh->GetNumVertices();
	UINT indexCount = d3dxMesh->GetNumFaces() * 3;


	d3dxMesh = GenerateTangentFrame(d3dxMesh, vertexCount, indexCount, indexBufferFormat, d3d9Device);

	vertexStride = d3dxMesh->GetNumBytesPerVertex();
	vertexCount = d3dxMesh->GetNumVertices();
	indexCount = d3dxMesh->GetNumFaces() * 3;


	UINT nameLen = strlen(model->GetMesh(meshIdx)->Name);
	if (nameLen > 0)
	{
		WCHAR* name = new WCHAR[nameLen + 1];
		AnsiToWString(model->GetMesh(meshIdx)->Name, name, nameLen + 1);

		if (!output->write((const char*)&nameLen, sizeof(UINT)))
		{
			delete[] name;
			return E_FAIL;
		}

		if (!output->write((const char*)name, sizeof(WCHAR) * nameLen))
		{
			delete[] name;
			return E_FAIL;
		}

		delete[] name;
	}
	else
	{
		WCHAR* name = new WCHAR[MAX_PATH];
		swprintf_s(name, MAX_PATH, L"Mesh %u", meshIdx);

		UINT len = wcslen(name);

		if (!output->write((const char*)&len, sizeof(UINT)))
		{
			delete[] name;
			return E_FAIL;
		}

		if (!output->write((const char*)name, sizeof(WCHAR) * len))
		{
			delete[] name;
			return E_FAIL;
		}

		delete[] name;
	}

	// Copy over the vertex data
	Vertex* vertices = NULL;
	V_RETURN(d3dxMesh->LockVertexBuffer(0, (LPVOID*)&vertices));

	if (!output->write((const char*)&vertexCount, sizeof(UINT)))
	{
		return E_FAIL;
	}

	if (!output->write((const char*)vertices, sizeof(Vertex) * vertexCount))
	{
		return E_FAIL;
	}

	V_RETURN(d3dxMesh->UnlockVertexBuffer());

	void* finalIndices = NULL;
	V_RETURN(d3dxMesh->LockIndexBuffer(0, &finalIndices));

	if (!output->write((const char*)&indexBufferFormat, sizeof(DXGI_FORMAT)))
	{
		return E_FAIL;
	}

	if (!output->write((const char*)&indexCount, sizeof(UINT)))
	{
		return E_FAIL;
	}
	
	UINT finalIndexSize = (indexBufferFormat == DXGI_FORMAT_R32_UINT) ? 4 : 2;
	if (!output->write((const char*)finalIndices, finalIndexSize * indexCount))
	{
		return E_FAIL;
	}
	
	// Copy in the subset info
	DWORD subsetCount = 0;
	V_RETURN(d3dxMesh->GetAttributeTable(NULL, &subsetCount));
	D3DXATTRIBUTERANGE* attributeTable = new D3DXATTRIBUTERANGE[subsetCount];
	V_RETURN(d3dxMesh->GetAttributeTable(attributeTable, &subsetCount));

	if (!output->write((const char*)&subsetCount, sizeof(UINT)))
	{
		return E_FAIL;
	}

	MeshPart part;
	for(UINT i = 0; i < subsetCount; ++i)
	{
		part.VertexStart = attributeTable[i].VertexStart;
		part.IndexStart = attributeTable[i].FaceStart * 3;
		part.IndexCount = attributeTable[i].FaceCount * 3;
		part.MaterialIndex = attributeTable[i].AttribId;

		if (!output->write((const char*)&part, sizeof(MeshPart)))
		{
			return E_FAIL;
		}
	}

	delete[] attributes;
	delete[] attributeTable;
	SAFE_RELEASE(d3dxMesh);

	return S_OK;
}

HRESULT Mesh::Create(ID3D11Device* device, std::istream* input, Mesh** output)
{
	HRESULT hr;

	Mesh* result = new Mesh();

	UINT nameLen;
	input->read((char*)&nameLen, sizeof(UINT));

	result->_name = new WCHAR[nameLen + 1];
	result->_name[nameLen] = '\0';
	input->read((char*)result->_name, nameLen * sizeof(WCHAR));

	// Read the vertices and create the vertex buffer
	result->_vertexStride = sizeof(Vertex);
	input->read((char*)&result->_vertexCount, sizeof(UINT));

	Vertex* verts = new Vertex[result->_vertexCount];
	input->read((char*)verts, sizeof(Vertex) * result->_vertexCount);

	Collision::ComputeBoundingAxisAlignedBoxFromPoints(&result->_boundingBox, result->_vertexCount, 
		(XMFLOAT3*)verts, sizeof(Vertex));

	D3D11_BUFFER_DESC vbDesc =
	{
		result->_vertexCount * sizeof(Vertex),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER,
		0,
		0
	};

	D3D11_SUBRESOURCE_DATA vbInitData;
	vbInitData.pSysMem = verts;
	vbInitData.SysMemPitch = 0;
	vbInitData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&vbDesc, &vbInitData, &result->_vertexBuffer);
	delete[] verts;
	if (FAILED(hr))
	{
		delete result;
		return E_FAIL;
	}

	// Read the indices and create the index buffer
	input->read((char*)&result->_indexBufferFormat, sizeof(DXGI_FORMAT));
	input->read((char*)&result->_indexCount, sizeof(UINT));

	UINT indexSize = (result->_indexBufferFormat == DXGI_FORMAT_R32_UINT) ? sizeof(uint32_t) : sizeof(uint16_t);

	BYTE* indices = new BYTE[result->_indexCount * indexSize];
	input->read((char*)indices, result->_indexCount * indexSize);

	D3D11_BUFFER_DESC ibDesc =
	{
		result->_indexCount * indexSize,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_INDEX_BUFFER,
		0,
		0
	};

	D3D11_SUBRESOURCE_DATA ibInitData;
	ibInitData.pSysMem = indices;
	ibInitData.SysMemPitch = 0;
	ibInitData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&ibDesc, &ibInitData, &result->_indexBuffer);
	delete[] indices;
	if (FAILED(hr))
	{
		delete result;
		return E_FAIL;
	}

	// Read the meshparts
	input->read((char*)&result->_meshPartCount, sizeof(UINT));

	result->_meshParts = new MeshPart[result->_meshPartCount];
	input->read((char*)result->_meshParts, result->_meshPartCount* sizeof(MeshPart));
	
	// Prepare the input layout
	const D3D11_INPUT_ELEMENT_DESC layout_mesh[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	result->_inputElementCount = ARRAYSIZE(layout_mesh);

	result->_inputElements = new D3D11_INPUT_ELEMENT_DESC[result->_inputElementCount];
	memcpy(result->_inputElements, layout_mesh, sizeof(D3D11_INPUT_ELEMENT_DESC) * result->_inputElementCount);

	*output = result;
	return S_OK;
}
