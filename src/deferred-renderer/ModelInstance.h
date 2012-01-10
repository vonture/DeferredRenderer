#pragma once

#include "PCH.h"
#include "IHasContent.h"
#include "IDragable.h"
#include "Model.h"
#include "xnaCollision.h"

class ModelInstance : public IHasContent, public IDragable
{
private:
	const WCHAR* _path;
	Model* _model;
	
	XMFLOAT4X4 _prevWorld;
	XMFLOAT4X4 _world;
	XMFLOAT3 _position;
	float _scale;
	XMFLOAT4 _orientation;

	OrientedBox _transformedMainOrientedBox;
	OrientedBox* _transformedMeshOrientedBoxes;

	AxisAlignedBox _transformedMainAxisBox;
	AxisAlignedBox* _transformedMeshAxisBoxes;
	
	bool _dirty;
	void clean();

public:
	ModelInstance(const WCHAR* path);

	const XMFLOAT3& GetPosition() const { return _position; }
	float GetScale() const { return _scale; }
	const XMFLOAT4& GetOrientation() const { return _orientation; }

	void SetPosition(const XMFLOAT3& pos);
	void SetScale(float scale);
	void SetOrientation(const XMFLOAT4& orientation);
	const XMFLOAT4X4& GetWorld();
	const XMFLOAT4X4& GetPreviousWorld() const;
	
	const AxisAlignedBox& GetMeshAxisAlignedBox(UINT meshIdx);
	const AxisAlignedBox& GetAxisAlignedBox();

	const OrientedBox& GetMeshOrientedBox(UINT meshIdx);
	const OrientedBox& GetOrientedBox();

	UINT GetModelMeshCount() const { return _model->GetMeshCount(); }

	Model* GetModel() { return _model; }

	void StoreWorld();

	void FillBoundingObjectSet(BoundingObjectSet* set);
	bool RayIntersect(const Ray& ray, float* dist);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, 
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, 
		IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};