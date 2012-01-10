#pragma once

#include "PCH.h"
#include "PostProcess.h"
#include "IHasContent.h"
#include "BoundingObjectSet.h"
#include "xnaCollision.h"

class BoundingObjectPostProcess : public PostProcess
{
private:
	struct BOUNDING_OBJECT_VERTEX
	{
		XMFLOAT3 Position;
	};

	struct CB_BOUNDING_OBJECT_PROPERTIES
	{
		XMFLOAT4X4 WorldViewProjection;
	};

	struct CB_BOUNDING_OBJECT_COLOR
	{
		XMFLOAT4 Color;
	};
	
	BoundingObjectSet _objects;

	ID3D11Buffer* _boxVB;
	ID3D11Buffer* _boxIB;

	ID3D11Buffer* _sphereVB;
	static const UINT SPHERE_POINT_COUNT = 99;
	
	ID3D11InputLayout* _inputLayout;

	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader* _pixelShader;
		
	ID3D11Buffer* _wvpConstantBuffer;
	ID3D11Buffer* _colorConstantBuffer;

	XMFLOAT4 _boColor;

	void fillRingVB(BOUNDING_OBJECT_VERTEX* buffer, UINT startIdx, UINT numSegments, const XMFLOAT3& Origin, 
		const XMFLOAT3& MajorAxis, const XMFLOAT3& MinorAxis);

public:
	BoundingObjectPostProcess();
	~BoundingObjectPostProcess();

	void Add(const AxisAlignedBox& aabb);
	void Add(const OrientedBox& obb);
	void Add(const Sphere& sphere);
	void Add(const Frustum& frust);
	void Add(BoundingObjectSet* objectSet);

	void SetColor(const XMFLOAT4& color) { _boColor = color; }
	const XMFLOAT4& GetColor() const { return _boColor; }

	void Clear();

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, ParticleBuffer* pBuffer,LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};