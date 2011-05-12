#include "DXUT.h"
#include "ModelInstance.h"

ModelInstance::ModelInstance(const WCHAR* path) 
	: _path(path), _position(0.0f, 0.0f, 0.0f), _scale(1.0f, 1.0f, 1.0f), _orientation(0.0f, 0.0f, 0.0f, 1.0f),
	  _dirty(true)
{
}

ModelInstance::~ModelInstance()
{
	SAFE_DELETE(_path);
}

void ModelInstance::clean()
{
	D3DXMATRIX translate, scale, rotate;
	
	D3DXMatrixTranslation(&translate, _position.x, _position.y, _position.z);
	D3DXMatrixScaling(&scale, _scale.x, _scale.y, _scale.z);
	D3DXMatrixRotationQuaternion(&rotate, &_orientation);

	D3DXMatrixMultiply(&_world, &rotate, &scale);
	D3DXMatrixMultiply(&_world, &_world, &translate);

	BoundingBox::Transform(&_worldBB, &_modelBB, &_world);

	_dirty = false;
}

HRESULT ModelInstance::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	V_RETURN(_mesh.Create(pd3dDevice, _path));

	// Build the model space bb
	D3DXVECTOR3 max = D3DXVECTOR3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	D3DXVECTOR3 min = D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);

	// Combine all of the mesh's min and maxes
	for (UINT i = 0; i < _mesh.GetNumMeshes(); i++)
	{
		D3DXVECTOR3 meshMin = _mesh.GetMeshBBoxCenter(i) - _mesh.GetMeshBBoxExtents(i);
		D3DXVECTOR3 meshMax = _mesh.GetMeshBBoxCenter(i) + _mesh.GetMeshBBoxExtents(i);;

		D3DXVec3Minimize(&min, &min, &meshMin);
		D3DXVec3Maximize(&max, &max, &meshMin);
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