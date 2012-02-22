#pragma once

#include "PCH.h"
#include "ContentType.h"
#include "Particle.h"
#include "xnaCollision.h"

class ParticleSystem : public ContentType
{
private:
	std::wstring _name;

	ID3D11ShaderResourceView* _diffuse;
	ID3D11ShaderResourceView* _normal;

	float _spread;
	XMFLOAT3 _positionVariance;

	float _spawnRate;
	float _lifeSpan;

	float _startSize;
	float _endSize;
	float _sizeExponent;

	float _startSpeed;
	float _endSpeed;
	float _speedExponent;
	float _speedVariance;

	float _rollAmount;
	float _windFalloff;

	XMFLOAT3 _direction;
	XMFLOAT3 _directionVariance;

	XMFLOAT4 _initialColor;
	XMFLOAT4 _finalColor;
	float _fadeExponent;
	float _alphaPower;

public:
	ParticleSystem();
	~ParticleSystem();

	const std::wstring& GetName() const { return _name; }

	static HRESULT Compile(ID3D11Device* device, const std::wstring& fileName, std::ostream& output);
	static HRESULT Create(ID3D11Device* device, std::istream& input, ParticleSystem** output);

	void Destroy();

	UINT GetMaxSimultaneousParticles() const { return (UINT)(_lifeSpan / _spawnRate);  }
	float GetSpawnRate() const { return _spawnRate; }

	ID3D11ShaderResourceView* GetDiffuseSRV();
	ID3D11ShaderResourceView* GetNormalSRV();

	void SpawnParticle(const XMFLOAT3& emitterPos, const XMFLOAT4& emitterRot, float emitterScale,
		Particle* outParticle);
	void AdvanceParticles(float dt, const XMFLOAT3& wind, const XMFLOAT3& gravity,
		Particle* particles, UINT count, AxisAlignedBox* outBounds);
};