#include "PCH.h"
#include "Mesh.h"
#include "Logger.h"

Mesh::Mesh()
    : _indexBuffer(NULL), _indexCount(0), _vertexBuffer(NULL), _vertexCount(0), _vertexStride(0),
    _meshParts(NULL), _meshPartCount(0), _inputElements(NULL), _alphaCutoutEnabled(true),
    _drawBackFaces(false)
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

HRESULT Mesh::CompileFromASSIMPMesh( ID3D11Device* device, const aiScene* scene, UINT meshIdx, std::ostream& output )
{
    aiMesh* mesh = scene->mMeshes[meshIdx];

    if (!mesh->HasFaces() || !mesh->HasNormals() || !mesh->HasPositions() ||
        !mesh->HasTangentsAndBitangents() || mesh->GetNumUVChannels() == 0)
    {
        return E_FAIL;
    }

    // Copy the name
    WCHAR* name;
    if (mesh->mName.length > 0)
    {
        name = new WCHAR[mesh->mName.length + 1];
        AnsiToWString(mesh->mName.data, name, mesh->mName.length + 1);
    }
    else
    {
        name = new WCHAR[MAX_PATH];
        swprintf_s(name, MAX_PATH, L"Mesh %u", meshIdx);
    }
    WriteWStringToStream(name, output);
    delete[] name;

    UINT uvChannel = 0;

    UINT vertexCount = mesh->mNumVertices;
    WriteDataTostream(vertexCount, output);

    Vertex vert;
    for (UINT i = 0; i < vertexCount; i++)
    {
        vert.Position = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vert.TexCoord = XMFLOAT2(mesh->mTextureCoords[uvChannel][i].x, mesh->mTextureCoords[uvChannel][i].y);
        vert.Normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        vert.Tangent = XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        vert.Bitangent = XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);

        WriteDataTostream(vert, output);
    }

    // Create the indices
    DXGI_FORMAT format = DXGI_FORMAT_R32_UINT;
    WriteDataTostream(format, output);

    UINT indexCount = mesh->mNumFaces * 3;
    WriteDataTostream(indexCount, output);

    for (UINT i = 0; i < mesh->mNumFaces; i++)
    {
        WriteDataArrayTostream(mesh->mFaces[i].mIndices, 3, output);
    }

    MeshPart part;
    part.IndexStart = 0;
    part.IndexCount = indexCount;
    part.MaterialIndex = mesh->mMaterialIndex;
    part.VertexStart = 0;

    UINT partCount = 1;
    WriteDataTostream(partCount, output);
    WriteDataTostream(part, output);

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
}

HRESULT Mesh::CompileFromSDKMeshMesh( ID3D11Device* device, IDirect3DDevice9* d3d9Device, const std::wstring& modelPath, SDKMesh* model, UINT meshIdx, std::ostream& output )
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

    WCHAR* name;
    UINT nameLen = strlen(model->GetMesh(meshIdx)->Name);
    if (nameLen > 0)
    {
        name = new WCHAR[nameLen + 1];
        AnsiToWString(model->GetMesh(meshIdx)->Name, name, nameLen + 1);
    }
    else
    {
        name = new WCHAR[MAX_PATH];
        swprintf_s(name, MAX_PATH, L"Mesh %u", meshIdx);
    }
    WriteWStringToStream(name, output);
    delete[] name;

    // Copy over the vertex data
    Vertex* vertices = NULL;
    V_RETURN(d3dxMesh->LockVertexBuffer(0, (LPVOID*)&vertices));

    WriteDataTostream(vertexCount, output);
    WriteDataArrayTostream(vertices, vertexCount, output);

    V_RETURN(d3dxMesh->UnlockVertexBuffer());

    BYTE* finalIndices = NULL;
    V_RETURN(d3dxMesh->LockIndexBuffer(0, (void**)&finalIndices));

    WriteDataTostream(indexBufferFormat, output);
    WriteDataTostream(indexCount, output);

    UINT finalIndexSize = (indexBufferFormat == DXGI_FORMAT_R32_UINT) ? 4 : 2;
    WriteDataArrayTostream(finalIndices, indexCount * finalIndexSize, output);

    // Copy in the subset info
    DWORD subsetCount = 0;
    V_RETURN(d3dxMesh->GetAttributeTable(NULL, &subsetCount));
    D3DXATTRIBUTERANGE* attributeTable = new D3DXATTRIBUTERANGE[subsetCount];
    V_RETURN(d3dxMesh->GetAttributeTable(attributeTable, &subsetCount));

    WriteDataTostream(subsetCount, output);

    MeshPart part;
    for(UINT i = 0; i < subsetCount; ++i)
    {
        part.VertexStart = attributeTable[i].VertexStart;
        part.IndexStart = attributeTable[i].FaceStart * 3;
        part.IndexCount = attributeTable[i].FaceCount * 3;
        part.MaterialIndex = attributeTable[i].AttribId;

        WriteDataTostream(part, output);
    }

    delete[] attributes;
    delete[] attributeTable;
    SAFE_RELEASE(d3dxMesh);

    return S_OK;
}

HRESULT Mesh::Create(ID3D11Device* device, std::istream& input, Mesh** output)
{
    HRESULT hr;

    Mesh* result = new Mesh();

    result->_name = ReadWStringFromStream(input);

    // Read the vertices and create the vertex buffer
    result->_vertexStride = sizeof(Vertex);
    ReadDataFromStream(result->_vertexCount, input);

    Vertex* verts = new Vertex[result->_vertexCount];
    ReadDataArrayFromStream(verts, result->_vertexCount, input);

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
    ReadDataFromStream(result->_indexBufferFormat, input);
    ReadDataFromStream(result->_indexCount, input);

    UINT indexSize = (result->_indexBufferFormat == DXGI_FORMAT_R32_UINT) ? sizeof(uint32_t) : sizeof(uint16_t);

    BYTE* indices = new BYTE[result->_indexCount * indexSize];
    ReadDataArrayFromStream(indices, result->_indexCount * indexSize, input);

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
    ReadDataFromStream(result->_meshPartCount, input);

    result->_meshParts = new MeshPart[result->_meshPartCount];
    ReadDataArrayFromStream(result->_meshParts, result->_meshPartCount, input);

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