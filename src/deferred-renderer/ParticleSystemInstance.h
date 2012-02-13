#pragma once

#include "PCH.h"
#include "IHasContent.h"
#include "IDragable.h"
#include "ParticleSystem.h"
#include "Particle.h"
#include "Camera.h"

class ParticleSystemInstance : public IHasContent, public IDragable
{
private:
	const WCHAR* _path;

	ParticleSystem* _system;

	ID3D11Buffer* _vb;
	UINT _vertexStride;

	XMFLOAT4X4 _world;
	XMFLOAT3 _position;
	float _scale;
	XMFLOAT4 _orientation;
	
	AxisAlignedBox _aabb;

	Particle* _particles;

	struct PARTICLE_SORT_INFO
	{
		UINT Particle;
		float Depth;
	};
	PARTICLE_SORT_INFO* _particleSortSpace;

	UINT _particleCount;
	UINT _particleIndex;
	bool _rolledOver;

	float _spawnTimer;

	float _selectRadius;
	
	bool _worldDirty;
	void clean();

	void DepthSort(PARTICLE_SORT_INFO* parts, int low, int high);

public:
	ParticleSystemInstance(const WCHAR* path);

	const XMFLOAT3& GetPosition() const;
	float GetScale() const;
	const XMFLOAT4& GetOrientation() const;
	const XMFLOAT4X4& GetWorld();

	void SetPosition(const XMFLOAT3& pos);
	void SetScale(float scale);
	void SetOrientation(const XMFLOAT4& orientation);

	UINT GetParticleCount() const;
	Particle* GetParticles();

	ParticleSystem* GetParticleSystem();

	void Reset();
	void AdvanceSystem(const XMFLOAT3& wind, const XMFLOAT3& gravity, float dt);

	void FillBoundingObjectSet(BoundingObjectSet* set);
	bool RayIntersect(const Ray& ray, float* dist);

	ID3D11ShaderResourceView* GetDiffuseSRV();
	ID3D11ShaderResourceView* GetNormalSRV();
	ID3D11Buffer* GetParticleVertexBuffer(ID3D11DeviceContext* pd3d11DeviceContext, Camera* camera);
	UINT GetVertexStride() const;

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, 
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice(ContentManager* pContentManager);

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, 
		IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};
