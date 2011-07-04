#pragma once

#include "Defines.h"
#include "PostProcess.h"
#include "FullscreenQuad.h"

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

class DepthOfFieldPostProcess : public PostProcess
{
private:
	FullscreenQuad _fsQuad;
	
	float _focalDistance;
	float _falloffStart;
	float _falloffEnd;
	float _cocScale;

	ID3D11Texture2D* _cocSizeTexture;
	ID3D11RenderTargetView* _cocSizeRTV;
	ID3D11ShaderResourceView* _cocSizeSRV;
	
	ID3D11PixelShader* _cocSizePS;
	ID3D11PixelShader* _dofBlurPS;

	ID3D11Buffer* _propertiesBuffer;

public:
	DepthOfFieldPostProcess();
	~DepthOfFieldPostProcess();

	float GetFocalDistance() const { return _focalDistance; }
	void SetFocalDistance(float dist) { _focalDistance = max(dist, 0.0f); }

	float GetFocalFalloffStart() const { return _falloffStart; }
	float GetFocalFalloffEnd() const { return _falloffEnd; }
	float SetFocalFalloffs(float start, float end) { _falloffStart = max(start, 0.0f); _falloffEnd = max(end, _falloffStart); }

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};