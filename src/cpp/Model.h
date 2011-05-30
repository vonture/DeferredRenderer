#pragma once

#include "Defines.h"
#include "BoundingObjects.h"

struct Material
{
    XMFLOAT3 AmbientColor;
    XMFLOAT3 DiffuseColor;
    XMFLOAT3 EmissiveColor;
    
	XMFLOAT3 SpecularColor;
	float SpecularPower;

    float Alpha;

    ID3D11ShaderResourceView* DiffuseMap;
    ID3D11ShaderResourceView* NormalMap;
	ID3D11ShaderResourceView* SpecularMap;
};

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
	ID3D11Buffer* _vertexBuffer;

	MeshPart _meshParts[];
	UINT _numMeshParts;

	BoundingSphere _boundingSphere;
	BoundingBox _boundingBox;

public:
	Mesh();
	~Mesh();

	const MeshPart& GetMeshPart(UINT idx) const { return _meshParts[idx]; }
	UINT GetMeshPartCount() const { return _numMeshParts; }

	const ID3D11Buffer* GetVertexBuffer() const { return _vertexBuffer; }
	const ID3D11Buffer* GetIndexBuffer() const { return _indexBuffer; }

	const BoundingBox& GetBoundingBox() const { return _boundingBox; }
	const BoundingSphere& GetBoundingSphere() const { return _boundingSphere; }
};