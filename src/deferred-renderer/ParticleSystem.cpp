#include "PCH.h"
#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
	: _name(NULL), _diffuse(NULL), _normal(NULL), _spread(0), _positionVariance(0.0f, 0.0f, 0.0f), 
	  _spawnRate(0), _lifeSpan(0), _startSize(0),
	  _endSize(0), _sizeExponent(0), _startSpeed(0), _endSpeed(0), _speedExponent(0), _rollAmount(0), 
	  _windFalloff(0), _direction(0.0f, 0.0f, 0.0f), _directionVariance(0.0f, 0.0f, 0.0f),
	  _initialColor(0.0f, 0.0f, 0.0f, 0.0f), _finalColor(0.0f, 0.0f, 0.0f, 0.0f), _fadeExponent(0)
{
}

ParticleSystem::~ParticleSystem()
{
}

HRESULT ParticleSystem::CreateFromFile(ID3D11Device* device, const WCHAR* fileName)
{
	HRESULT hr;

	_name = L"Smoke";

	const WCHAR* diffusePath = L"C:\\Programming\\deferred-renderer\\src\\deferred-renderer\\media\\Particles\\smoke_diffuse.dds";
	V_RETURN(D3DX11CreateShaderResourceViewFromFile(device, diffusePath, NULL, NULL, &_diffuse, NULL));

	const WCHAR* normalPath = L"C:\\Programming\\deferred-renderer\\src\\deferred-renderer\\media\\Particles\\smoke_normal.dds";
	V_RETURN(D3DX11CreateShaderResourceViewFromFile(device, normalPath, NULL, NULL, &_normal, NULL));
	
	_spread = 1.0f;	
	_positionVariance = XMFLOAT3(0.1f, 0.0f, 0.1f);

	_spawnRate = 0.05f;
	_lifeSpan = 4.0f;

	_startSize = 0.8f;
	_endSize = 2.0f;
	_sizeExponent = 1.4f;

	_startSpeed = 4.0f;
	_endSpeed = 0.01f;
	_speedExponent = 0.6f;

	_rollAmount = Pi / 8.0f;
	_windFalloff = 0.0f;

	_direction = XMFLOAT3(0.0f, 1.0f, 0.0f);
	_directionVariance = XMFLOAT3(0.25f, 0.0f, 0.25f);

	_initialColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	_finalColor = XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f);
	_fadeExponent = 0.9f;

	return S_OK;
}

void ParticleSystem::Destroy()
{
	SAFE_RELEASE(_diffuse);
	SAFE_RELEASE(_normal);
	SAFE_DELETE(_name);
}

void ParticleSystem::SpawnParticle(const XMFLOAT3& emitterPos, const XMFLOAT4& emitterRot, float emitterScale,
	Particle* outParticle)
{
	outParticle->Position.x = emitterPos.x + (RandomBetween(-1.0f, 1.0f) * _spread * emitterScale * _positionVariance.x);
	outParticle->Position.y = emitterPos.y + (RandomBetween(-1.0f, 1.0f) * _spread * emitterScale *_positionVariance.y);
	outParticle->Position.z = emitterPos.z + (RandomBetween(-1.0f, 1.0f) * _spread * emitterScale *_positionVariance.z);

	XMVECTOR rotVec = XMLoadFloat4(&emitterRot);
	XMMATRIX rotMat = XMMatrixRotationQuaternion(rotVec);

	XMVECTOR velocity = XMVectorSet(
		_direction.x + (RandomBetween(-1.0f, 1.0f) * _directionVariance.x),
		_direction.y + (RandomBetween(-1.0f, 1.0f) * _directionVariance.y),
		_direction.z + (RandomBetween(-1.0f, 1.0f) * _directionVariance.z),
		1.0f);
	XMStoreFloat3(&outParticle->Velocity, XMVector3Transform(XMVector3Normalize(velocity), rotMat));
	
	outParticle->Color = _initialColor;

	outParticle->Radius = _startSize;
	
	outParticle->Scale = emitterScale;

	outParticle->Life = 0.0f;

	outParticle->Rotation = RandomBetween(-1.0f, 1.0f) * Pi;
	outParticle->RotationRate = RandomBetween(-1.0f, 1.0f) * _rollAmount;
}

void ParticleSystem::AdvanceParticles(float dt, const XMFLOAT3& wind, const XMFLOAT3& gravity,
	Particle* particles, UINT count)
{
	for (UINT i = 0; i < count; i++)
	{
		Particle* part = &particles[i];

		part->Life += dt;
		
		float t = part->Life / _lifeSpan;
		
		float fSizeLerp = pow(t, _sizeExponent);
		part->Radius = (fSizeLerp * _endSize + (1.0f - fSizeLerp) * _startSize) * part->Scale;

		float fSpeedLerp =  pow(t, _speedExponent);
		float fSpeed = (fSpeedLerp * _endSpeed + (1.0f - fSpeedLerp) * _startSpeed) * part->Scale;
		part->Position.x += ((part->Velocity.x * fSpeed) + wind.x) * dt;
		part->Position.y += ((part->Velocity.y * fSpeed) + wind.y) * dt;
		part->Position.z += ((part->Velocity.z * fSpeed) + wind.z) * dt;

		part->Rotation += part->RotationRate * dt;

		float fFadeLerp = pow(t, _fadeExponent);
		part->Color.x = fFadeLerp * _initialColor.x + (1.0f - fFadeLerp) * _finalColor.x;
		part->Color.y = fFadeLerp * _initialColor.y + (1.0f - fFadeLerp) * _finalColor.y;
		part->Color.z = fFadeLerp * _initialColor.z + (1.0f - fFadeLerp) * _finalColor.z;
		part->Color.w = fFadeLerp * _initialColor.w + (1.0f - fFadeLerp) * _finalColor.w;
	}
}

ID3D11ShaderResourceView* ParticleSystem::GetDiffuseSRV()
{
	return _diffuse;
}

ID3D11ShaderResourceView* ParticleSystem::GetNormalSRV()
{
	return _normal;
}
