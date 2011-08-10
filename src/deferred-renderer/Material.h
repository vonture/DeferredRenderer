#pragma once

#include "PCH.h"
#include "SDKmesh.h"
#include "aiScene.h"

struct CB_MATERIAL_PROPERTIES
{
	_DECLSPEC_ALIGN_16_ XMFLOAT3 AmbientColor;
    _DECLSPEC_ALIGN_16_ XMFLOAT3 DiffuseColor;
    _DECLSPEC_ALIGN_16_ XMFLOAT3 EmissiveColor;    
	_DECLSPEC_ALIGN_16_ XMFLOAT3 SpecularColor;
	float SpecularPower;
    float Alpha;
	XMFLOAT2 Padding;
};

class Material
{
private:
    XMFLOAT3 _ambientColor;
    XMFLOAT3 _diffuseColor;
    XMFLOAT3 _emissiveColor;    
	XMFLOAT3 _specularColor;
	float _specularPower;

    float _alpha;

    ID3D11ShaderResourceView* _diffuseSRV;
    ID3D11ShaderResourceView* _normalSRV;
	ID3D11ShaderResourceView* _specularSRV;

	ID3D11Buffer* _propertiesBuffer;

	HRESULT createPropertiesBuffer(ID3D11Device* device);

public:
	Material();
	~Material();

	const XMFLOAT3& GetAmbientColor() const { return _ambientColor; }
	const XMFLOAT3& GetDiffuseColor() const { return _diffuseColor; }
	const XMFLOAT3& GetEmissiveColor() const { return _emissiveColor; }
	const XMFLOAT3& GetSpecularColor() const { return _specularColor; }
	float GetSpecularPower() const { return _specularPower; }
	float GetAlpha() const { return _alpha; }

	ID3D11ShaderResourceView* GetDiffuseSRV() const { return _diffuseSRV; }
	ID3D11ShaderResourceView* GetNormalSRV() const { return _normalSRV; }
	ID3D11ShaderResourceView* GetSpecularSRV() const { return _specularSRV; }

	ID3D11Buffer* GetPropertiesBuffer() const { return _propertiesBuffer; }

	HRESULT CreateFromSDKMeshMaterial(ID3D11Device* device, const WCHAR* modelPath, SDKMesh* model,
		UINT materialIdx);
	HRESULT CreateFromASSIMPMaterial(ID3D11Device* device, const WCHAR* modelPath, const aiScene* scene,
		UINT materialIdx);
	void Destroy();
};