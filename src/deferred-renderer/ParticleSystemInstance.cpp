#include "PCH.h"
#include "ParticleSystemInstance.h"
#include "ParticleSystemLoader.h"

ParticleSystemInstance::ParticleSystemInstance(const WCHAR* path)
	: _path(path), _system(NULL), _vb(NULL), _position(0.0f, 0.0f, 0.0f), _scale(1.0f),
	  _orientation(0.0f, 0.0f, 0.0f, 1.0f), _worldDirty(true), _particles(NULL), _particleSortSpace(NULL),
	  _particleCount(0), _particleIndex(0), _rolledOver(false), _spawnTimer(0), _vertexStride(0), 
	  _selectRadius(1.5f)
{
}

void ParticleSystemInstance::clean()
{
	// Load the position and orientation into vectors
	XMVECTOR position = XMLoadFloat3(&_position);
	XMVECTOR orientation = XMLoadFloat4(&_orientation);

	// Create the matrices for translation, rotation and scaling
	XMMATRIX translate = XMMatrixTranslationFromVector(position);
	XMMATRIX rotate = XMMatrixRotationQuaternion(orientation);
	XMMATRIX scale = XMMatrixScaling(_scale, _scale, _scale);

	// Store the world matrix
	XMMATRIX world =  XMMatrixMultiply(scale, XMMatrixMultiply(rotate, translate));
	XMStoreFloat4x4(&_world, world);
}

const XMFLOAT3& ParticleSystemInstance::GetPosition() const
{
	return _position;
}

float ParticleSystemInstance::GetScale() const
{
	return _scale;
}

const XMFLOAT4& ParticleSystemInstance::GetOrientation() const
{
	return _orientation;
}

const XMFLOAT4X4& ParticleSystemInstance::GetWorld()
{
	if (_worldDirty)
	{
		clean();
	}

	return _world;
}

void ParticleSystemInstance::SetPosition(const XMFLOAT3& pos)
{
	_position = pos;
	_worldDirty = true;
}

void ParticleSystemInstance::SetScale(float scale)
{
	_scale = scale;
	_worldDirty = true;
}

void ParticleSystemInstance::SetOrientation(const XMFLOAT4& orientation)
{
	_orientation = orientation;
	_worldDirty = true;
}

UINT ParticleSystemInstance::GetParticleCount() const
{
	return (_rolledOver) ? _particleCount : _particleIndex;
}

Particle* ParticleSystemInstance::GetParticles()
{
	return _particles;
}

ParticleSystem* ParticleSystemInstance::GetParticleSystem()
{
	return _system;
}

void ParticleSystemInstance::Reset()
{
	_particleIndex = 0; 
	_spawnTimer = 0.0f;
	_rolledOver = false;
}

void ParticleSystemInstance::FillBoundingObjectSet( BoundingObjectSet* set )
{
	set->AddAxisAlignedBox(_aabb);
}

bool ParticleSystemInstance::RayIntersect(const Ray& ray, float* dist)
{
	XMVECTOR rayOrigin = XMLoadFloat3(&ray.Origin);
	XMVECTOR rayDir = XMLoadFloat3(&ray.Direction);

	return !Collision::IntersectPointAxisAlignedBox(rayOrigin, &_aabb) &&
		Collision::IntersectRayAxisAlignedBox(rayOrigin, rayDir, &_aabb, dist);
}

void ParticleSystemInstance::AdvanceSystem(const XMFLOAT3& wind, const XMFLOAT3& gravity, float dt)
{
	_spawnTimer -= dt;

	// Spawn new particles
	while (_spawnTimer < 0.0f)
	{
		_system->SpawnParticle(_position, _orientation, _scale, &_particles[_particleIndex]);
		
		_particleIndex = (_particleIndex + 1) % _particleCount;
		if (!_rolledOver && _particleIndex == 0)
		{
			_rolledOver = true;
		}

		_spawnTimer += _system->GetSpawnRate();
	}

	_system->AdvanceParticles(dt, wind, gravity, _particles, GetParticleCount(), &_aabb);
}

void ParticleSystemInstance::DepthSort(PARTICLE_SORT_INFO* parts, int low, int high)
{
	//  low is the lower index, high is the upper index
	//  of the region of array a that is to be sorted
	int i = low, j = high;
	float h;
	int index;
	float x = parts[(low + high) / 2].Depth;

	//  partition
	do
	{    
		while (parts[i].Depth > x)
		{
			i++;
		}
		while (parts[j].Depth < x)
		{
			j--;
		}

		if (i <= j)
		{
			h = parts[i].Depth; parts[i].Depth = parts[j].Depth; parts[j].Depth = h;
			index = parts[i].Particle; parts[i].Particle = parts[j].Particle; parts[j].Particle = index;
			
			i++;
			j--;
		}
	} while (i <= j);

	//  recursion
	if (low < j)
	{
		DepthSort(parts, low, j);
	}
	if (i < high) 
	{
		DepthSort(parts, i, high);
	}
}

ID3D11Buffer* ParticleSystemInstance::GetParticleVertexBuffer(ID3D11DeviceContext* pd3d11DeviceContext, 
	Camera* camera)
{
	HRESULT hr;

	UINT partCount = GetParticleCount();	

	// Prepare the particles to be sorted
	XMFLOAT3 camForward = camera->GetForward();
	for (UINT i = 0; i < partCount; i++)
	{
		_particleSortSpace[i].Particle = i;

		// Dot product
		_particleSortSpace[i].Depth = 
			(camForward.x * _particles[i].Position.x) + 
			(camForward.y * _particles[i].Position.y) +
			(camForward.z * _particles[i].Position.z);
	}

	// Sort the particles
	DepthSort(_particleSortSpace, 0, partCount - 1);

	// Copy particles in sorted order to the buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	V(pd3d11DeviceContext->Map(_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	ParticleVertex* bufferData = (ParticleVertex*)mappedResource.pData;

	for (UINT i = 0; i < partCount; i++)
	{
		bufferData[i].Position =			_particles[_particleSortSpace[i].Particle].Position;
		bufferData[i].PreviousPosition =	_particles[_particleSortSpace[i].Particle].PreviousPosition;
		bufferData[i].Color =				_particles[_particleSortSpace[i].Particle].Color;
		bufferData[i].Radius =				_particles[_particleSortSpace[i].Particle].Radius;
		bufferData[i].PreviousRadius =		_particles[_particleSortSpace[i].Particle].PreviousRadius;
		bufferData[i].Rotation =			_particles[_particleSortSpace[i].Particle].Rotation;
		bufferData[i].PreviousRotation =	_particles[_particleSortSpace[i].Particle].PreviousRotation;
	}

	pd3d11DeviceContext->Unmap(_vb, 0);

	return _vb;
}

UINT ParticleSystemInstance::GetVertexStride() const
{
	return _vertexStride;
}

ID3D11ShaderResourceView* ParticleSystemInstance::GetDiffuseSRV()
{
	return _system->GetDiffuseSRV();
}

ID3D11ShaderResourceView* ParticleSystemInstance::GetNormalSRV()
{
	return _system->GetNormalSRV();
}

HRESULT ParticleSystemInstance::OnD3D11CreateDevice(ID3D11Device* pd3dDevice,
	ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
	HRESULT hr;

	V_RETURN(pContentManager->LoadContent(pd3dDevice, _path, (ParticleSystemOptions*)NULL, &_system));

	_particleCount = _system->GetMaxSimultaneousParticles();
	_particles = new Particle[_particleCount];
	_particleSortSpace = new PARTICLE_SORT_INFO[_particleCount];
	if (!_particles || !_particleSortSpace)
	{
		SAFE_DELETE_ARRAY(_particles);
		SAFE_DELETE_ARRAY(_particleSortSpace);
		return E_FAIL;
	}

	D3D11_BUFFER_DESC vbDesc = 
	{
		_particleCount * sizeof(ParticleVertex), // INT ByteWidth;
		D3D11_USAGE_DYNAMIC, // D3D11_USAGE Usage;
		D3D11_BIND_VERTEX_BUFFER, // UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE, // UINT CPUAccessFlags;
		0, // UINT MiscFlags;
		0, // UINT StructureByteStride;
	};
	V_RETURN(pd3dDevice->CreateBuffer(&vbDesc, NULL, &_vb));
	_vertexStride = sizeof(ParticleVertex);

	_particleIndex = 0;
	_spawnTimer = 0;
	_rolledOver = false;
	
	return S_OK;
}


void ParticleSystemInstance::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
	SAFE_CM_RELEASE(pContentManager, _system);
	SAFE_RELEASE(_vb);

	SAFE_DELETE_ARRAY(_particles);
	SAFE_DELETE_ARRAY(_particleSortSpace);
	_particleCount = 0;
}

HRESULT ParticleSystemInstance::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice,
	ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void ParticleSystemInstance::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{

}
