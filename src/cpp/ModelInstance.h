#pragma once

#include "Defines.h"
#include "xnaCollision.h"
#include "IHasContent.h"
#include "Model.h"
#include <map>

class ModelInstance : public IHasContent
{
private:
	const WCHAR* _path;
	Model _model;
	
	XMMATRIX _world;
	XMVECTOR _position;
	float _scale;
	XMVECTOR _orientation;

	OrientedBox _transformedMainBox;
	OrientedBox* _transformedMeshBoxes;

	VOID TransformOrientedBox( OrientedBox* pOut, const OrientedBox* pIn, FLOAT Scale, FXMVECTOR Rotation,
                           FXMVECTOR Translation );
	bool _dirty;
	void clean();

public:
	ModelInstance(const WCHAR* path);
	~ModelInstance();

	const XMVECTOR& GetPosition() const { return _position; }
	float GetScale() const { return _scale; }
	const XMVECTOR& GetOrientation() const { return _orientation; }

	void SetPosition(const XMVECTOR& pos)
	{
		_position = pos;

		_dirty = true;
	}

	void SetScale(float scale)
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
	
	const OrientedBox& GetMeshBoundingBox(UINT meshIdx)
	{
		if (_dirty)
		{
			clean();
		}

		return _transformedMeshBoxes[meshIdx];
	}

	const OrientedBox& GetBoundingBox() 
	{ 
		if (_dirty)
		{
			clean();
		}

		return _transformedMainBox;
	}

	UINT GetModelMeshCount() const { return _model.GetMeshCount(); }

	Model* GetModel() { return &_model; }

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};