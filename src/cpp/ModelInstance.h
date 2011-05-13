#pragma once

#include "Defines.h"
#include "SDKmesh.h"
#include "IHasContent.h"
#include "BoundingObjects.h"
#include <map>

class ModelInstance : public IHasContent
{
private:
	const WCHAR* _path;
	CDXUTSDKMesh _mesh;
	
	BoundingBox _modelBB;
	BoundingBox _worldBB;

	XMMATRIX _world;
	XMVECTOR _position;
	XMVECTOR _scale;
	XMVECTOR _orientation;

	bool _dirty;
	void clean();

public:
	ModelInstance(const WCHAR* path);
	~ModelInstance();

	void SetPosition(const XMVECTOR& pos)
	{
		_position = pos;

		_dirty = true;
	}

	const XMVECTOR* GetScale() const
	{ 
		return &_scale; 
	}

	void SetScale(const XMVECTOR& scale)
	{
		_scale = scale;

		_dirty = true;
	}

	const XMVECTOR* GetPosition() const
	{ 
		return &_position; 
	}

	void SetOrientation(const XMVECTOR& orientation)
	{
		_orientation = orientation;

		_dirty = true;
	}

	const XMVECTOR* GetOrientation() const
	{ 		
		return &_orientation;
	}

	const XMMATRIX* GetWorld() 
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

		return &_worldBB;
	}

	CDXUTSDKMesh* GetMesh()
	{
		return &_mesh;
	}

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};