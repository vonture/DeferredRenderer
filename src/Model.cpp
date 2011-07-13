#include "Model.h"
#include "SDKmesh.h"
#include "SDKmisc.h"

Model::Model()
	: _meshes(NULL), _meshCount(0), _materials(NULL), _materialCount(0)
{
}

Model::~Model()
{
}

IDirect3DDevice9* createD3D9Device()
{
	HRESULT hr;

    // Create a D3D9 device (would make it NULL, but PIX doesn't seem to like that)
    IDirect3D9* d3d9;
    d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

    D3DPRESENT_PARAMETERS pp;
    pp.BackBufferWidth = 1;
    pp.BackBufferHeight = 1;
    pp.BackBufferFormat = D3DFMT_X8R8G8B8;
    pp.BackBufferCount = 1;
    pp.MultiSampleType = D3DMULTISAMPLE_NONE;
    pp.MultiSampleQuality = 0;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = GetDesktopWindow();
    pp.Windowed = true;
    pp.Flags = 0;
    pp.FullScreen_RefreshRateInHz = 0;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    pp.EnableAutoDepthStencil = false;

    IDirect3DDevice9* d3d9Device = NULL;
    V(d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &d3d9Device));

    return d3d9Device;
}

HRESULT Model::CreateFromSDKMeshFile(ID3D11Device* device, LPCWSTR fileName)
{
	HRESULT hr;
	SDKMesh sdkMesh;

	WCHAR resolvedPath[MAX_PATH];
	V_RETURN(DXUTFindDXSDKMediaFileCch(resolvedPath, MAX_PATH, fileName));
    V_RETURN(sdkMesh.Create(resolvedPath));

	WCHAR directory[MAX_PATH];
	GetDirectoryFromFileNameW(resolvedPath, directory, MAX_PATH);
	
	// Make materials
    _materialCount = sdkMesh.GetNumMaterials();
	_materials = new Material[_materialCount];
    for (UINT i = 0; i < _materialCount; i++)
    {
        V_RETURN(_materials[i].CreateFromSDKMeshMaterial(device, directory, &sdkMesh, i));
    }
	
	// Create a d3d9 device for loading the meshes
	IDirect3DDevice9* d3d9device = createD3D9Device();

	// Copy the meshes
	_meshCount = sdkMesh.GetNumMeshes();
	_meshes = new Mesh[_meshCount];
    for (UINT i = 0; i < _meshCount; i++)
	{
		V_RETURN(_meshes[i].CreateFromSDKMeshMesh(device, d3d9device, directory, &sdkMesh, i));
	}

	SAFE_RELEASE(d3d9device);

	// Done with the sdk mesh, free all it's resources
	sdkMesh.Destroy();

	// Compute the overall bounding box
	if (_meshCount > 0)
	{
		_boundingBox = _meshes[0].GetAxisAlignedBox();

		for (UINT i = 1; i < _meshCount; i++)
		{
			AxisAlignedBox aaBox = _meshes[i].GetAxisAlignedBox();

			Collision::MergeAxisAlignedBoxes(&_boundingBox, &_boundingBox, &aaBox);
		}
	}

	return S_OK;
}

void Model::Destroy()
{
	for (UINT i = 0; i < _materialCount; i++)
	{
		_materials[i].Destroy();
	}
	SAFE_DELETE_ARRAY(_materials);
	_materialCount = 0;

	for (UINT i = 0; i < _meshCount; i++)
    {
		_meshes[i].Destroy();
	}
	SAFE_DELETE_ARRAY(_meshes);
	_meshCount = 0;
}

HRESULT Model::Render(ID3D11DeviceContext* context,  UINT materialBufferSlot, UINT diffuseSlot,
	UINT normalSlot, UINT specularSlot)
{
	HRESULT hr;

	for (UINT i = 0; i < _meshCount; i++)
	{
		V_RETURN(RenderMesh(context, i, materialBufferSlot, diffuseSlot, normalSlot, specularSlot));
	}

	return S_OK;
}

HRESULT Model::RenderMesh(ID3D11DeviceContext* context, UINT meshIdx, UINT materialBufferSlot,
	UINT diffuseSlot, UINT normalSlot, UINT specularSlot)
{
	ID3D11Buffer* vertexBuffers[1] = { _meshes[meshIdx].GetVertexBuffer() };
	UINT strides[1] = { _meshes[meshIdx].GetVertexStride() };
	UINT offsets[1] = { 0 };

	context->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
	context->IASetIndexBuffer(_meshes[meshIdx].GetIndexBuffer(), _meshes[meshIdx].GetIndexBufferFormat(), 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	for (UINT i = 0; i < _meshes[meshIdx].GetMeshPartCount(); i++)
	{
		MeshPart part = _meshes[meshIdx].GetMeshPart(i);
		UINT matIdx = part.MaterialIndex;

		if (materialBufferSlot != INVALID_BUFFER_SLOT)
		{
			ID3D11Buffer* buf = _materials[matIdx].GetPropertiesBuffer();
			context->PSSetConstantBuffers(materialBufferSlot, 1, &buf);
		}
		if (diffuseSlot != INVALID_SAMPLER_SLOT)
		{
			ID3D11ShaderResourceView* srv = _materials[matIdx].GetDiffuseSRV();
			context->PSSetShaderResources(diffuseSlot, 1, &srv);
		}
		if (normalSlot != INVALID_SAMPLER_SLOT)
		{
			ID3D11ShaderResourceView* srv = _materials[matIdx].GetNormalSRV();
			context->PSSetShaderResources(normalSlot, 1, &srv);
		}
		if (specularSlot != INVALID_SAMPLER_SLOT)
		{
			ID3D11ShaderResourceView* srv = _materials[matIdx].GetSpecularSRV();
			context->PSSetShaderResources(specularSlot, 1, &srv);
		}

        context->DrawIndexed(part.IndexCount, part.IndexStart, 0);
	}

	return S_OK;
}