#include "PCH.h"
#include "ModelInstance.h"
#include "ModelLoader.h"

ModelInstance::ModelInstance(const WCHAR* path) 
	: _model(NULL), _path(path), _transformedMeshOrientedBoxes(NULL), _transformedMeshAxisBoxes(NULL),
	  _position(0.0f, 0.0f, 0.0f), _scale(1.0f), _orientation(0.0f, 0.0f, 0.0f, 1.0f), _dirty(true)
{
}

void ModelInstance::clean()
{
	// Load the position and orientation into vectors
	XMVECTOR position = XMLoadFloat3(&_position);
	XMVECTOR orientation = XMLoadFloat4(&_orientation);

	// Create the matrices for translation, rotation and scaling
	XMMATRIX translate = XMMatrixTranslationFromVector(position);
	XMMATRIX rotate = XMMatrixRotationQuaternion(orientation);
	XMMATRIX scale = XMMatrixScaling(_scale, _scale, _scale);

	// Store the world matrix
	XMMATRIX world =  XMMatrixMultiply(scale, XMMatrixMultiply(rotate, translate));
	XMStoreFloat4x4(&_world, world);
	
	// Create the bounding boxes...

	// Create an oriented box that sits in the same location as the model's boxes
	// and then transform it
	_transformedMainAxisBox = _model->GetAxisAlignedBox();	
	_transformedMainOrientedBox.Center = _transformedMainAxisBox.Center;
	_transformedMainOrientedBox.Extents = _transformedMainAxisBox.Extents;
	XMStoreFloat4(&_transformedMainOrientedBox.Orientation, XMQuaternionIdentity());

	Collision::TransformAxisAlignedBox(&_transformedMainAxisBox, &_transformedMainAxisBox, 
		_scale, orientation, position);
	Collision::TransformOrientedBox(&_transformedMainOrientedBox, &_transformedMainOrientedBox, 
		_scale, orientation, position);

	UINT meshCount = _model->GetMeshCount();
	for (UINT i = 0; i < meshCount; i++)
	{
		_transformedMeshAxisBoxes[i] = _model->GetMeshAxisAlignedBox(i);
		_transformedMeshOrientedBoxes[i].Center = _transformedMeshAxisBoxes[i].Center;
		_transformedMeshOrientedBoxes[i].Extents = _transformedMeshAxisBoxes[i].Extents;
		XMStoreFloat4(&_transformedMeshOrientedBoxes[i].Orientation, XMQuaternionIdentity());

		Collision::TransformAxisAlignedBox(&_transformedMeshAxisBoxes[i], &_transformedMeshAxisBoxes[i],
			_scale, orientation, position);
		Collision::TransformOrientedBox(&_transformedMeshOrientedBoxes[i], &_transformedMeshOrientedBoxes[i],
			_scale, orientation, position);
	}

	_dirty = false;
}

void ModelInstance::SetPosition(const XMFLOAT3& pos)
{
	_position = pos;

	_dirty = true;
}

void ModelInstance::SetScale(float scale)
{
	_scale = scale;

	_dirty = true;
}

void ModelInstance::SetOrientation(const XMFLOAT4& orientation)
{
	_orientation = orientation;

	_dirty = true;
}

const XMFLOAT4X4& ModelInstance::GetWorld() 
{ 
	if (_dirty)
	{
		clean();
	}

	return _world;
}

const XMFLOAT4X4& ModelInstance::GetPreviousWorld() const
{
	return _prevWorld;
}

const AxisAlignedBox& ModelInstance::GetMeshAxisAlignedBox(UINT meshIdx)
{
	if (_dirty)
	{
		clean();
	}

	return _transformedMeshAxisBoxes[meshIdx];
}

const AxisAlignedBox& ModelInstance::GetAxisAlignedBox() 
{ 
	if (_dirty)
	{
		clean();
	}

	return _transformedMainAxisBox;
}

const OrientedBox& ModelInstance::GetMeshOrientedBox(UINT meshIdx)
{
	if (_dirty)
	{
		clean();
	}

	return _transformedMeshOrientedBoxes[meshIdx];
}

const OrientedBox& ModelInstance::GetOrientedBox() 
{ 
	if (_dirty)
	{
		clean();
	}

	return _transformedMainOrientedBox;
}

void ModelInstance::FillBoundingObjectSet(BoundingObjectSet* set)
{
	for (UINT i = 0; i < _model->GetMeshCount(); i++)
	{
		set->AddOrientedBox(_transformedMeshOrientedBoxes[i]);
	}
}

void ModelInstance::StoreWorld()
{
	_prevWorld = GetWorld();
}

bool ModelInstance::RayIntersect(const Ray& ray, float* dist)
{
	if (_dirty)
	{
		clean();
	}

	XMVECTOR rayOrigin = XMLoadFloat3(&ray.Origin);
	XMVECTOR rayDir = XMLoadFloat3(&ray.Direction);	

	float minDist = FLT_MAX;
	bool found = false;
	for (UINT i = 0; i < _model->GetMeshCount(); i++)
	{
		float dist;
		if (!Collision::IntersectPointOrientedBox(rayOrigin, &_transformedMeshOrientedBoxes[i]) &&
			Collision::IntersectRayOrientedBox(rayOrigin, rayDir, &_transformedMeshOrientedBoxes[i], &dist) &&
			dist < minDist)
		{
			minDist = dist;
			found = true;
		}
	}

	*dist = minDist;
	return found;
}


HRESULT ModelInstance::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	
	V_RETURN(pContentManager->LoadContent(pd3dDevice, _path, (ModelOptions*)NULL, &_model));
	
	UINT meshCount = _model->GetMeshCount();
	_transformedMeshOrientedBoxes = new OrientedBox[meshCount];
	_transformedMeshAxisBoxes = new AxisAlignedBox[meshCount];

	_dirty = true;

	return S_OK;
}

void ModelInstance::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
	SAFE_CM_RELEASE(pContentManager, _model);

	SAFE_DELETE_ARRAY(_transformedMeshOrientedBoxes);
	SAFE_DELETE_ARRAY(_transformedMeshAxisBoxes);

	_dirty = true;
}

HRESULT ModelInstance::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void ModelInstance::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{
}
