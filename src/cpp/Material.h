#pragma once

#include "Defines.h"
#include "SDKmesh.h"

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

	HRESULT CreateFromSDKMeshMaterial(ID3D11Device* device, CDXUTSDKMesh* model, UINT materialIdx);
	void Destroy();
};