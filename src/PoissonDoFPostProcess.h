#pragma once

#include "PCH.h"
#include "PostProcess.h"

class PoissonDoFPostProcess : public PostProcess
{
private:	
	float _focalDistance;
	float _falloffStart;
	float _falloffEnd;
	float _cocScale;
	UINT _sampleCountIndex;

	static const UINT NUM_DOF_SAMPLE_COUNTS = 5;
	static const UINT DOF_SAMPLE_COUNTS[NUM_DOF_SAMPLE_COUNTS];
	static const UINT DOF_SAMPLE_COUNT_MAX = 64;

	ID3D11PixelShader* _ps;

	ID3D11Buffer* _propertiesBuffer;
	struct CB_DOF_PROPERTIES
	{
		float CameraNearClip;
		float CameraFarClip;
		float FocalDistance;
		float FocalFalloffNear;
		float FocalFalloffFar;
		float CircleOfConfusionScale;
		XMFLOAT2 Padding;
	};

	ID3D11Buffer* _sampleBuffers[NUM_DOF_SAMPLE_COUNTS];

public:
	PoissonDoFPostProcess();
	~PoissonDoFPostProcess();

	float GetFocalDistance() const { return _focalDistance; }
	void SetFocalDistance(float dist) { _focalDistance = max(dist, 0.0f); }

	float GetFocalFalloffStart() const { return _falloffStart; }
	float GetFocalFalloffEnd() const { return _falloffEnd; }
	void SetFocalFalloffs(float start, float end) { _falloffStart = max(start, 0.0f); _falloffEnd = max(end, _falloffStart); }

	float GetCircleOfConfusionScale() const { return _cocScale; }
	void SetCircleOfConfusionScale(float scale) { _cocScale = max(scale, 0.0f); }

	UINT GetSampleCount() const { return DOF_SAMPLE_COUNTS[_sampleCountIndex]; }
	UINT GetSampleCountIndex() const { return _sampleCountIndex; }
	void SetSampleCountIndex(UINT idx) { _sampleCountIndex = min(idx, NUM_DOF_SAMPLE_COUNTS - 1); }

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};