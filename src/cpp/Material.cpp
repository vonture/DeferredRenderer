#include "Material.h"

Material::Material()
	: _ambientColor(0.0f, 0.0f, 0.0f), _diffuseColor(0.0f, 0.0f, 0.0f), _emissiveColor(0.0f, 0.0f, 0.0f),
	  _specularColor(0.0f, 0.0f, 0.0f), _specularPower(0.0f), _alpha(1.0f), _diffuseSRV(NULL), 
	  _normalSRV(NULL), _specularSRV(NULL)
{
}

Material::~Material()
{
}

HRESULT Material::CreateFromSDKMeshMaterial(ID3D11Device* device, CDXUTSDKMesh* model, UINT materialIdx)
{
	SDKMESH_MATERIAL* sdkmat = model->GetMaterial(materialIdx);

	_ambientColor = XMFLOAT3(sdkmat->Ambient.x, sdkmat->Ambient.y, sdkmat->Ambient.z);
	_diffuseColor = XMFLOAT3(sdkmat->Diffuse.x, sdkmat->Diffuse.y, sdkmat->Diffuse.z);
	_specularColor = XMFLOAT3(sdkmat->Specular.x, sdkmat->Specular.y, sdkmat->Specular.z);
	_emissiveColor = XMFLOAT3(sdkmat->Emissive.x, sdkmat->Emissive.y, sdkmat->Emissive.z);
    _alpha = sdkmat->Diffuse.w;
    _specularPower = sdkmat->Power;

	// Not sure why the pointers point to 1 when the srv's fail to load
	if (sdkmat->pDiffuseRV11 != NULL && sdkmat->Force64_4 != 1)
	{
		_diffuseSRV = sdkmat->pDiffuseRV11;			
		_diffuseSRV->AddRef();
	}

	if (sdkmat->pNormalRV11 && sdkmat->Force64_5 != 1)
	{
		_normalSRV = sdkmat->pNormalRV11;			
		_normalSRV->AddRef();
	}

	if (sdkmat->pSpecularRV11 && sdkmat->Force64_6 != 1)
	{
		_specularSRV = sdkmat->pSpecularRV11;			
		_specularSRV->AddRef();
	}

	return S_OK;
}

void Material::Destroy()
{
	SAFE_RELEASE(_diffuseSRV);
	SAFE_RELEASE(_normalSRV);
	SAFE_RELEASE(_specularSRV);
}