#pragma once

#include "Defines.h"
#include "Application.h"
#include "Renderer.h"
#include "FirstPersonCamera.h"
#include "KeyboardState.h"
#include "MouseState.h"

#include "HDRPostProcess.h"
#include "SkyPostProcess.h"
#include "AntiAliasPostProcess.h"
#include "AmbientOcclusionPostProcess.h"
#include "DepthOfFieldPostProcess.h"
#include "MotionBlurPostProcess.h"

class DeferredRendererApplication : public Application
{
private:
	Renderer _renderer;
	FirstPersonCamera _camera;

	ModelInstance _scene;

	HDRPostProcess _hdrPP;
	SkyPostProcess _skyPP;
	AntiAliasPostProcess _aaPP;
	AmbientOcclusionPostProcess _aoPP;
	DepthOfFieldPostProcess _dofPP;
	MotionBlurPostProcess _motionBlurPP;

	bool _aoEnabled;
	bool _aaEnabled;
	bool _hdrEnabled;

protected:
	void OnPreparingDeviceSettings(DeviceManager* deviceManager);

public:
	DeferredRendererApplication();
	~DeferredRendererApplication();

	void OnFrameMove(double totalTime, float dt);
	HRESULT OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};