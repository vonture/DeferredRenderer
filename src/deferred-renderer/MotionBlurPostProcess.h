#pragma once

#include "PCH.h"
#include "PostProcess.h"

class MotionBlurPostProcess : public PostProcess
{
private:
	ID3D11Buffer* _propertiesBuffer;

	struct CB_MOTIONBLUR_PROPERTIES
	{
		XMFLOAT4 Padding;
	};

public:
	MotionBlurPostProcess();

	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, ID3D11ShaderResourceView* src,
		ID3D11RenderTargetView* dstRTV, Camera* camera, GBuffer* gBuffer, LightBuffer* lightBuffer);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};