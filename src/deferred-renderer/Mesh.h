#pragma once

#include "PCH.h"
#include "ContentType.h"
#include "SDKmesh.h"
#include "aiScene.h"
#include "xnaCollision.h"

struct MeshPart
{
	UINT VertexStart;
    UINT IndexStart;
    UINT IndexCount;
    UINT MaterialIndex;
};

class Mesh : public ContentType
{
private:
	WCHAR* _name;

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

	bool _alphaCutoutEnabled;
	bool _drawBackFaces;

	struct Vertex
	{
		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT2 TexCoord;
		XMFLOAT3 Tangent;
		XMFLOAT3 Bitangent;
	};

	static D3DXVECTOR3 Perpendicular(const D3DXVECTOR3& vec);

	static void CreateInputElements(D3DVERTEXELEMENT9* declaration, D3D11_INPUT_ELEMENT_DESC** output,
		UINT* elementCount);

	static ID3DXMesh* GenerateTangentFrame(ID3DXMesh* mesh, UINT numVertices, UINT numIndices,
		DXGI_FORMAT indexType, IDirect3DDevice9* d3d9Device);

public:
	Mesh();
	~Mesh();

	const MeshPart* GetMeshPart(UINT idx) const { return &_meshParts[idx]; }
	UINT GetMeshPartCount() const { return _meshPartCount; }

	ID3D11Buffer* GetVertexBuffer() const { return _vertexBuffer; }

	const UINT GetVertexStride() const { return _vertexStride; }
	
	DXGI_FORMAT GetIndexBufferFormat() const { return _indexBufferFormat; }
	ID3D11Buffer* GetIndexBuffer() const { return _indexBuffer; }

	const D3D11_INPUT_ELEMENT_DESC* GetInputLayout() const { return _inputElements; }
	UINT GetInputElementCount() const { return _inputElementCount; }

	const AxisAlignedBox& GetAxisAlignedBox() const { return _boundingBox; }

	bool GetAlphaCutoutEnabled() const { return _alphaCutoutEnabled; }
	bool GetDrawBackFaces() const { return _drawBackFaces; }

	const WCHAR* GetName() const { return _name; }

	void Destroy();

	static HRESULT CompileFromASSIMPMesh(ID3D11Device* device, const aiScene* scene, UINT meshIdx, std::ostream* output);
	static HRESULT CompileFromSDKMeshMesh(ID3D11Device* device, IDirect3DDevice9* d3d9Device, 
		const WCHAR* modelPath, SDKMesh* model,	UINT meshIdx, std::ostream* output);
	static HRESULT Create(ID3D11Device* device, std::istream* input, Mesh** output);
};
