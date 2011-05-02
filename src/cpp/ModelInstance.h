#pragma once

#include "DXUT.h"
#include "SDKmesh.h"
#include "IHasContent.h"
#include "BoundingObjects.h"

class ModelInstance : public IHasContent
{
private:
	const WCHAR* _path;
	CDXUTSDKMesh _mesh;
	
	BoundingBox _bb;

	D3DXMATRIX _world;
	D3DXVECTOR3 _position;
	D3DXVECTOR3 _scale;
	D3DXQUATERNION _orientation;

	bool _dirty;
	void clean();
	void buildBoundingBox();

public:
	ModelInstance(const WCHAR* path);
	~ModelInstance();

	void SetPosition(const D3DXVECTOR3& pos)
	{
		_position = pos;

		_dirty = true;
	}

	const D3DXVECTOR3* GetScale() const
	{ 
		return &_scale; 
	}

	void SetScale(const D3DXVECTOR3& scale)
	{
		_scale = scale;

		_dirty = true;
	}

	const D3DXVECTOR3* GetPosition() const
	{ 
		return &_position; 
	}

	void SetOrientation(const D3DXQUATERNION& orientation)
	{
		_orientation = orientation;

		_dirty = true;
	}

	const D3DXQUATERNION* GetOrientation() const
	{ 		
		return &_orientation;
	}

	const D3DXMATRIX* GetWorld() 
	{ 
		if (_dirty)
		{
			clean();
		}

		return &_world;
	}

	const BoundingBox* GetBounds()
	{
		if (_dirty)
		{
			clean();
		}

		return &_bb;
	}

	const CDXUTSDKMesh* GetMesh()
	{
		return &_mesh;
	}

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext);	
	void OnD3D11DestroyDevice(void* pUserContext);

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
	void OnD3D11ReleasingSwapChain(void* pUserContext);
};