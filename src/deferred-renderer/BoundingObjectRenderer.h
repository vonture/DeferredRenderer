#pragma once

#include "PCH.h"
#include "IHasContent.h"
#include "Camera.h"
#include "DeviceStates.h"
#include "xnaCollision.h"

struct BOUNDING_OBJECT_VERTEX
{
	XMFLOAT3 Position;
	XMFLOAT3 Color;
};

struct CB_BOUNDING_OBJECT_PROPERTIES
{
	XMFLOAT4X4 WorldViewProjection;
};

class BoundingObjectRenderer : public IHasContent
{
private:
	static const UINT MAX_BOUNDING_OBJECTS = 512;

	UINT _nextAABB;
	AxisAlignedBox _aabbs[MAX_BOUNDING_OBJECTS];

	UINT _nextOBB;
	OrientedBox _obbs[MAX_BOUNDING_OBJECTS];

	UINT _nextSphere;
	Sphere _spheres[MAX_BOUNDING_OBJECTS];

	UINT _nextFrust;
	Frustum _frustums[MAX_BOUNDING_OBJECTS];

	ID3D11Buffer* _boxVB;
	ID3D11Buffer* _boxIB;

	ID3D11Buffer* _sphereVB;
	static const UINT SPHERE_POINT_COUNT = 99;
	
	ID3D11InputLayout* _inputLayout;

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;
		
	ID3D11Buffer* _constantBuffer;

	void fillRingVB(BOUNDING_OBJECT_VERTEX* buffer, UINT startIdx, UINT numSegments, const XMFLOAT3& Origin, 
		const XMFLOAT3& MajorAxis, const XMFLOAT3& MinorAxis, const XMFLOAT3& Color );

	DepthStencilStates _dsStates;
	SamplerStates _samplerStates;
	BlendStates _blendStates;
	RasterizerStates _rasterStates;

protected:
	DepthStencilStates* GetDepthStencilStates()
	{
		return &_dsStates;
	}
	SamplerStates* GetSamplerStates()
	{
		return &_samplerStates;
	}
	BlendStates* GetBlendStates()
	{
		return &_blendStates;
	}
	RasterizerStates* GetRasterizerStates()
	{
		return &_rasterStates;
	}

public:
	BoundingObjectRenderer();

	void Add(const AxisAlignedBox& aabb);
	void Add(const OrientedBox& obb);
	void Add(const Sphere& sphere);
	void Add(const Frustum& frust);

	void Clear();

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, Camera* camera);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};