#include "PCH.h"
#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
	: _name(NULL), _diffuse(NULL), _normal(NULL), _spread(0), _positionVariance(0.0f, 0.0f, 0.0f), 
	  _spawnRate(0), _lifeSpan(0), _startSize(0),
	  _endSize(0), _sizeExponent(0), _startSpeed(0), _endSpeed(0), _speedExponent(0), _speedVariance(0),
	  _rollAmount(0), 
	  _windFalloff(0), _direction(0.0f, 0.0f, 0.0f), _directionVariance(0.0f, 0.0f, 0.0f),
	  _initialColor(0.0f, 0.0f, 0.0f, 0.0f), _finalColor(0.0f, 0.0f, 0.0f, 0.0f), _fadeExponent(0), 
	  _alphaPower(1.0f)
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
	_lifeSpan = 8.0f;

	_startSize = 0.6f;
	_endSize = 1.5f;
	_sizeExponent = 1.4f;

	_startSpeed = 2.0f;
	_endSpeed = 0.1f;
	_speedExponent = 0.6f;
	_speedVariance = 0.5f;

	_rollAmount = Pi / 8.0f;
	_windFalloff = 0.0f;

	_direction = XMFLOAT3(0.0f, 1.0f, 0.0f);
	_directionVariance = XMFLOAT3(0.2f, 0.0f, 0.2f);

	_initialColor = XMFLOAT4(1.0f, 0.3f, 0.25f, 1.0f);
	_finalColor = XMFLOAT4(0.5f, 1.0f, 0.5f, 0.7f);
	_fadeExponent = 0.9f;
	_alphaPower = 2.0f;

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
	outParticle->PreviousPosition = outParticle->Position;

	XMVECTOR rotVec = XMLoadFloat4(&emitterRot);
	XMMATRIX rotMat = XMMatrixRotationQuaternion(rotVec);

	XMVECTOR velocity = XMVectorSet(
		_direction.x + (RandomBetween(-1.0f, 1.0f) * _directionVariance.x),
		_direction.y + (RandomBetween(-1.0f, 1.0f) * _directionVariance.y),
		_direction.z + (RandomBetween(-1.0f, 1.0f) * _directionVariance.z),
		1.0f);
	XMStoreFloat3(&outParticle->Velocity, XMVector3Transform(XMVector3Normalize(velocity), rotMat));
	
	outParticle->SpeedPerc = 1.0f + (RandomBetween(-1.0f, 1.0f) * _speedVariance);

	outParticle->Color = _initialColor;

	outParticle->Radius = _startSize;
	outParticle->PreviousRadius = outParticle->Radius;

	outParticle->Scale = emitterScale;

	outParticle->Life = 0.0f;

	outParticle->Rotation = RandomBetween(-1.0f, 1.0f) * Pi;
	outParticle->PreviousRotation = outParticle->Rotation;
	outParticle->RotationRate = RandomBetween(-1.0f, 1.0f) * _rollAmount;
}

void ParticleSystem::AdvanceParticles(float dt, const XMFLOAT3& wind, const XMFLOAT3& gravity,
	Particle* particles, UINT count, AxisAlignedBox* outBounds)
{
	XMFLOAT3 minBound = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 maxBound = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (UINT i = 0; i < count; i++)
	{
		Particle* part = &particles[i];

		part->Life += dt;
		
		float t = part->Life / _lifeSpan;
		
		float fSizeLerp = pow(t, _sizeExponent);
		part->PreviousRadius = part->Radius;
		part->Radius = (fSizeLerp * _endSize + (1.0f - fSizeLerp) * _startSize) * part->Scale;

		float fSpeedLerp =  pow(t, _speedExponent);
		float fSpeed = (fSpeedLerp * _endSpeed + (1.0f - fSpeedLerp) * _startSpeed) * part->Scale * part->SpeedPerc;
		part->PreviousPosition = part->Position;
		part->Position.x += ((part->Velocity.x * fSpeed) + wind.x) * dt;
		part->Position.y += ((part->Velocity.y * fSpeed) + wind.y) * dt;
		part->Position.z += ((part->Velocity.z * fSpeed) + wind.z) * dt;

		part->PreviousRotation = part->Rotation;
		part->Rotation += part->RotationRate * dt;

		float fFadeLerp = pow(t, _fadeExponent);
		part->Color.x = fFadeLerp * _finalColor.x + (1.0f - fFadeLerp) * _initialColor.x;
		part->Color.y = fFadeLerp * _finalColor.y + (1.0f - fFadeLerp) * _initialColor.y;
		part->Color.z = fFadeLerp * _finalColor.z + (1.0f - fFadeLerp) * _initialColor.z;
		part->Color.w = (1.0f - powf(2 * fFadeLerp - 1.0f, _alphaPower)) *
			(fFadeLerp * _finalColor.w + (1.0f - fFadeLerp) * _initialColor.w);

		minBound.x = min(minBound.x, part->Position.x - part->Radius);
		minBound.y = min(minBound.y, part->Position.y - part->Radius);
		minBound.z = min(minBound.z, part->Position.z - part->Radius);

		maxBound.x = max(maxBound.x, part->Position.x + part->Radius);
		maxBound.y = max(maxBound.y, part->Position.y + part->Radius);
		maxBound.z = max(maxBound.z, part->Position.z + part->Radius);
	}

	outBounds->Center.x = (maxBound.x + minBound.x) * 0.5f;
	outBounds->Center.y = (maxBound.y + minBound.y) * 0.5f;
	outBounds->Center.z = (maxBound.z + minBound.z) * 0.5f;

	outBounds->Extents.x = (maxBound.x - minBound.x) * 0.5f;
	outBounds->Extents.y = (maxBound.y - minBound.y) * 0.5f;
	outBounds->Extents.z = (maxBound.z - minBound.z) * 0.5f;
}

ID3D11ShaderResourceView* ParticleSystem::GetDiffuseSRV()
{
	return _diffuse;
}

ID3D11ShaderResourceView* ParticleSystem::GetNormalSRV()
{
	return _normal;
}
