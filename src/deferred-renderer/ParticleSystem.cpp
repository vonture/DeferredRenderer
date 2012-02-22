#include "PCH.h"
#include "ParticleSystem.h"
#include "tinyxml.h"

ParticleSystem::ParticleSystem()
	: _diffuse(NULL), _normal(NULL), _spread(0), _positionVariance(0.0f, 0.0f, 0.0f), 
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

void ParticleSystem::Destroy()
{
	SAFE_RELEASE(_diffuse);
	SAFE_RELEASE(_normal);
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

	outParticle->Rotation = 0.0f;//RandomBetween(-1.0f, 1.0f) * Pi;
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

HRESULT readFloatFromXML(TiXmlElement* root, const std::string& name, float& out)
{
	TiXmlElement* element = root->FirstChildElement(name.c_str());
	if (!element)
	{
		return E_FAIL;
	}
	
	int result = sscanf_s(element->GetText(), "%f", &out);

	return (result == 1) ? S_OK : E_FAIL;
}

HRESULT readFloat3FromXML(TiXmlElement* root, const std::string& name, XMFLOAT3& out)
{
	TiXmlElement* element = root->FirstChildElement(name.c_str());
	if (!element)
	{
		return E_FAIL;
	}
	
	int result = sscanf_s(element->GetText(), "%f %f %f", &out.x, &out.y, &out.z);

	return (result == 3) ? S_OK : E_FAIL;
}

HRESULT readFloat4FromXML(TiXmlElement* root, const std::string& name, XMFLOAT4& out)
{
	TiXmlElement* element = root->FirstChildElement(name.c_str());
	if (!element)
	{
		return E_FAIL;
	}

	int result = sscanf_s(element->GetText(), "%f %f %f %f", &out.x, &out.y, &out.z, &out.w);

	return (result == 4) ? S_OK : E_FAIL;
}

HRESULT readWStringFromXML(TiXmlElement* root, const std::string& name, std::wstring& out)
{
	TiXmlElement* element = root->FirstChildElement(name.c_str());
	if (!element)
	{
		return E_FAIL;
	}
	
	out = AnsiToWString(element->GetText());

	return S_OK;
}


HRESULT ParticleSystem::Compile(ID3D11Device* device, const std::wstring& path,
	std::ostream& output)
{
	std::string sPath = WStringToAnsi(path);

	TiXmlDocument doc = TiXmlDocument(sPath);
	if (!doc.LoadFile())
	{
		return E_FAIL;
	}

	TiXmlElement* root = doc.FirstChildElement("particleSystem");
	if (!root)
	{
		return E_FAIL;
	}

	std::wstring name = AnsiToWString(root->Attribute("name"));

	std::wstring diffName;
	if (FAILED(readWStringFromXML(root, "diffuse", diffName)))
	{
		return E_FAIL;
	}
	std::wstring diffPath = GetDirectoryFromFileNameW(path) + diffName;

	std::wstring normName;
	if (FAILED(readWStringFromXML(root, "normal", normName)))
	{
		return E_FAIL;
	}
	std::wstring normPath = GetDirectoryFromFileNameW(path) + normName;

	float spread;
	if (FAILED(readFloatFromXML(root, "spread", spread)))
	{
		return E_FAIL;
	}

	XMFLOAT3 positionVariance;
	if (FAILED(readFloat3FromXML(root, "positionVariance", positionVariance)))
	{
		return E_FAIL;
	}

	float spawnRate;
	if (FAILED(readFloatFromXML(root, "spawnRate", spawnRate)))
	{
		return E_FAIL;
	}

	float lifeSpan;
	if (FAILED(readFloatFromXML(root, "lifeSpan", lifeSpan)))
	{
		return E_FAIL;
	}

	float startSize;
	if (FAILED(readFloatFromXML(root, "startSize", startSize)))
	{
		return E_FAIL;
	}

	float endSize;
	if (FAILED(readFloatFromXML(root, "endSize", endSize)))
	{
		return E_FAIL;
	}

	float sizeExponent;
	if (FAILED(readFloatFromXML(root, "sizeExponent", sizeExponent)))
	{
		return E_FAIL;
	}

	float startSpeed;
	if (FAILED(readFloatFromXML(root, "startSpeed", startSpeed)))
	{
		return E_FAIL;
	}

	float endSpeed;
	if (FAILED(readFloatFromXML(root, "endSpeed", endSpeed)))
	{
		return E_FAIL;
	}

	float speedExponent;
	if (FAILED(readFloatFromXML(root, "speedExponent", speedExponent)))
	{
		return E_FAIL;
	}

	float speedVariance;
	if (FAILED(readFloatFromXML(root, "speedVariance", speedVariance)))
	{
		return E_FAIL;
	}

	float rollAmount;
	if (FAILED(readFloatFromXML(root, "rollAmount", rollAmount)))
	{
		return E_FAIL;
	}

	float windFalloff;
	if (FAILED(readFloatFromXML(root, "windFalloff", windFalloff)))
	{
		return E_FAIL;
	}
	
	XMFLOAT3 direction;
	if (FAILED(readFloat3FromXML(root, "direction", direction)))
	{
		return E_FAIL;
	}

	XMFLOAT3 directionVariance;
	if (FAILED(readFloat3FromXML(root, "directionVariance", directionVariance)))
	{
		return E_FAIL;
	}

	XMFLOAT4 initialColor;
	if (FAILED(readFloat4FromXML(root, "initialColor", initialColor)))
	{
		return E_FAIL;
	}

	XMFLOAT4 finalColor;
	if (FAILED(readFloat4FromXML(root, "finalColor", finalColor)))
	{
		return E_FAIL;
	}

	float fadeExponent;
	if (FAILED(readFloatFromXML(root, "fadeExponent", fadeExponent)))
	{
		return E_FAIL;
	}

	float alphaPower;
	if (FAILED(readFloatFromXML(root, "alphaPower", alphaPower)))
	{
		return E_FAIL;
	}
	
	WriteWStringToStream(name, output);
	if (FAILED(WriteFileAndSizeToStream(diffPath, output)))
	{
		return E_FAIL;
	}
	if (FAILED(WriteFileAndSizeToStream(normPath, output)))
	{
		return E_FAIL;
	}
	WriteDataTostream(spread, output);
	WriteDataTostream(positionVariance, output);
	WriteDataTostream(spawnRate, output);
	WriteDataTostream(lifeSpan, output);
	WriteDataTostream(startSize, output);
	WriteDataTostream(endSize, output);
	WriteDataTostream(sizeExponent, output);
	WriteDataTostream(startSpeed, output);
	WriteDataTostream(endSpeed, output);
	WriteDataTostream(speedExponent, output);
	WriteDataTostream(speedVariance, output);
	WriteDataTostream(rollAmount, output);
	WriteDataTostream(windFalloff, output);
	WriteDataTostream(direction, output);
	WriteDataTostream(directionVariance, output);
	WriteDataTostream(initialColor, output);
	WriteDataTostream(finalColor, output);
	WriteDataTostream(fadeExponent, output);
	WriteDataTostream(alphaPower, output);
	
	return S_OK;
}

HRESULT ParticleSystem::Create(ID3D11Device* device, std::istream& input,
	ParticleSystem** output)
{
	ParticleSystem* system = new ParticleSystem();

	system->_name = ReadWStringFromStream(input);

	UINT size;
	BYTE* data;
	if (FAILED(ReadFileFromStream(input, &data, size)))
	{
		delete system;
		return E_FAIL;
	}
	if (FAILED(D3DX11CreateShaderResourceViewFromMemory(device, data, size, NULL, NULL,
			&system->_diffuse, NULL)))
	{
		delete system;
		delete[] data;
		return E_FAIL;
	}
	delete[] data;

	if (FAILED(ReadFileFromStream(input, &data, size)))
	{
		delete system;
		return E_FAIL;
	}
	if (FAILED(D3DX11CreateShaderResourceViewFromMemory(device, data, size, NULL, NULL,
		&system->_normal, NULL)))
	{
		delete system;
		delete[] data;
		return E_FAIL;
	}
	delete[] data;

	ReadDataFromStream(system->_spread, input);
	ReadDataFromStream(system->_positionVariance, input);
	ReadDataFromStream(system->_spawnRate, input);
	ReadDataFromStream(system->_lifeSpan, input);
	ReadDataFromStream(system->_startSize, input);
	ReadDataFromStream(system->_endSize, input);
	ReadDataFromStream(system->_sizeExponent, input);
	ReadDataFromStream(system->_startSpeed, input);
	ReadDataFromStream(system->_endSpeed, input);
	ReadDataFromStream(system->_speedExponent, input);
	ReadDataFromStream(system->_speedVariance, input);
	ReadDataFromStream(system->_rollAmount, input);
	ReadDataFromStream(system->_windFalloff, input);
	ReadDataFromStream(system->_direction, input);
	ReadDataFromStream(system->_directionVariance, input);
	ReadDataFromStream(system->_initialColor, input);
	ReadDataFromStream(system->_finalColor, input);
	ReadDataFromStream(system->_fadeExponent, input);
	ReadDataFromStream(system->_alphaPower, input);

	*output = system;
	return S_OK;
}
