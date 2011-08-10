#include "PCH.h"
#include "ModelInstance.h"

ModelInstance::ModelInstance(const WCHAR* path) 
	: _path(path), _transformedMeshOrientedBoxes(NULL), _transformedMeshAxisBoxes(NULL),
	  _position(0.0f, 0.0f, 0.0f), _scale(1.0f), _orientation(0.0f, 0.0f, 0.0f, 1.0f), _dirty(true)
{
}

ModelInstance::~ModelInstance()
{
}

void ModelInstance::clean()
{
	// Load the position and orientation into vectors
	XMVECTOR position = XMLoadFloat3(&_position);
	XMVECTOR orientation = XMLoadFloat4(&_orientation);

	// Create the matricies for translation, rotation and scaling
	XMMATRIX translate = XMMatrixTranslationFromVector(position);
	XMMATRIX rotate = XMMatrixRotationQuaternion(orientation);
	XMMATRIX scale = XMMatrixScaling(_scale, _scale, _scale);

	// Store the world matrix
	XMMATRIX world =  XMMatrixMultiply(scale, XMMatrixMultiply(rotate, translate));
	XMStoreFloat4x4(&_world, world);
	
	// Create the bounding boxes...

	// Create an oriented box that sits in the same location as the model's boxes
	// and then transform it
	_transformedMainAxisBox = _model.GetAxisAlignedBox();	
	_transformedMainOrientedBox.Center = _transformedMainAxisBox.Center;
	_transformedMainOrientedBox.Extents = _transformedMainAxisBox.Extents;
	XMStoreFloat4(&_transformedMainOrientedBox.Orientation, XMQuaternionIdentity());

	Collision::TransformAxisAlignedBox(&_transformedMainAxisBox, &_transformedMainAxisBox, 
		_scale, orientation, position);
	Collision::TransformOrientedBox(&_transformedMainOrientedBox, &_transformedMainOrientedBox, 
		_scale, orientation, position);

	UINT meshCount = _model.GetMeshCount();
	for (UINT i = 0; i < meshCount; i++)
	{
		_transformedMeshAxisBoxes[i] = _model.GetMeshAxisAlignedBox(i);
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

HRESULT ModelInstance::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	V_RETURN(_model.CreateFromFile(pd3dDevice, _path));
	
	UINT meshCount = _model.GetMeshCount();
	_transformedMeshOrientedBoxes = new OrientedBox[meshCount];
	_transformedMeshAxisBoxes = new AxisAlignedBox[meshCount];

	_dirty = true;

	return S_OK;
}

void ModelInstance::OnD3D11DestroyDevice()
{
	_model.Destroy();

	SAFE_DELETE_ARRAY(_transformedMeshOrientedBoxes);
	SAFE_DELETE_ARRAY(_transformedMeshAxisBoxes);

	_dirty = true;
}

HRESULT ModelInstance::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void ModelInstance::OnD3D11ReleasingSwapChain()
{
}