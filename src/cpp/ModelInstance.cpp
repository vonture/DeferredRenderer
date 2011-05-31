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
	XMMATRIX translate = XMMatrixTranslationFromVector(_position);
	XMMATRIX rotate = XMMatrixRotationRollPitchYawFromVector(_orientation);
	XMMATRIX scale = XMMatrixScalingFromVector(_scale);

	_world = XMMatrixMultiply(scale, XMMatrixMultiply(rotate, translate));
	
	//BoundingBox::Transform(&_worldBB, _modelBB, _world);
	//BoundingSphere::Tra

	_dirty = false;
}

HRESULT ModelInstance::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;
	V_RETURN(_model.CreateFromSDKMeshFile(pd3dDevice, _path));
	
	return S_OK;
}

void ModelInstance::OnD3D11DestroyDevice()
{
	_model.Destroy();
}

HRESULT ModelInstance::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                        const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void ModelInstance::OnD3D11ReleasingSwapChain()
{
}