#pragma once

#include "Defines.h"
#include "Model.h"
#include "IHasContent.h"
#include "BoundingObjects.h"
#include <map>

class ModelInstance : public IHasContent
{
private:
	const WCHAR* _path;
	Model _model;
	
	XMMATRIX _world;
	XMVECTOR _position;
	XMVECTOR _scale;
	XMVECTOR _orientation;

	bool _dirty;
	void clean();

public:
	ModelInstance(const WCHAR* path);
	~ModelInstance();

	const XMVECTOR& GetPosition() const { return _position; }
	const XMVECTOR& GetScale() const { return _scale; }
	const XMVECTOR& GetOrientation() const { return _orientation; }

	void SetPosition(const XMVECTOR& pos)
	{
		_position = pos;

		_dirty = true;
	}

	void SetScale(const XMVECTOR& scale)
	{
		_scale = scale;

		_dirty = true;
	}

	void SetOrientation(const XMVECTOR& orientation)
	{
		_orientation = orientation;

		_dirty = true;
	}

	const XMMATRIX& GetWorld() 
	{ 
		if (_dirty)
		{
			clean();
		}

		return _world;
	}
	
	Model* GetModel()
	{
		return &_model;
	}

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};