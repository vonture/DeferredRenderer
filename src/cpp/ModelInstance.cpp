#include "ModelInstance.h"

ModelInstance::ModelInstance(const WCHAR* path) 
	: _path(path), _dirty(true)
{
	_position = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	_scale = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	_orientation = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
}

ModelInstance::~ModelInstance()
{
}

void ModelInstance::clean()
{
	XMMATRIX translate = XMMatrixTranslation(XMVectorGetX(_position), XMVectorGetY(_position), XMVectorGetZ(_position));
	XMMATRIX rotate = XMMatrixRotationRollPitchYawFromVector(_orientation);
	XMMATRIX scale = XMMatrixScalingFromVector(_scale);

	_world = XMMatrixMultiply(rotate, XMMatrixMultiply(scale, translate));
	
	BoundingBox::Transform(&_worldBB, &_modelBB, &_world);

	_dirty = false;
}

HRESULT ModelInstance::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	V_RETURN(_mesh.Create(pd3dDevice, _path));

	// Build the model space bb
	XMVECTOR max = XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);
	XMVECTOR min = XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);

	// Combine all of the mesh's min and maxes
	for (UINT i = 0; i < _mesh.GetNumMeshes(); i++)
	{
		D3DXVECTOR3 meshMin = _mesh.GetMeshBBoxCenter(i) - _mesh.GetMeshBBoxExtents(i);
		D3DXVECTOR3 meshMax = _mesh.GetMeshBBoxCenter(i) + _mesh.GetMeshBBoxExtents(i);;

		min = XMVectorMin(min, XMVectorSet(meshMin.x, meshMin.y, meshMin.z, 1.0f));
		max = XMVectorMax(max, XMVectorSet(meshMax.x, meshMax.y, meshMax.z, 1.0f));
	}

	_modelBB = BoundingBox(min, max);

	return S_OK;
}

void ModelInstance::OnD3D11DestroyDevice()
{
	_mesh.Destroy();
}

HRESULT ModelInstance::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void ModelInstance::OnD3D11ReleasingSwapChain()
{
}