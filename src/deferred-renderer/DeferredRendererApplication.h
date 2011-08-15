#pragma once

#include "PCH.h"
#include "Application.h"
#include "Renderer.h"
#include "TestingCamera.h"
#include "KeyboardState.h"
#include "MouseState.h"

#include "HDRPostProcess.h"
#include "SkyPostProcess.h"
#include "MLAAPostProcess.h"
#include "SSAOPostProcess.h"
#include "HBAOPostProcess.h"
#include "DiscDoFPostProcess.h"
#include "MotionBlurPostProcess.h"
#include "UIPostProcess.h"
#include "FXAAPostProcess.h"

#include "DualParaboloidPointLightRenderer.h"
#include "CascadedDirectionalLightRenderer.h"
#include "SpotLightRenderer.h"

#include "ShaderLoader.h"
#include "TextureLoader.h"

#include "ConfigurationWindow.h"
#include "PostProcessSelectionPane.h"
#include "LogWindow.h"

class DeferredRendererApplication : public Application
{
private:
	Renderer _renderer;
	TestingCamera _camera;

	ModelInstance _scene;

	std::vector<IHasContent*> _contentHolders;

	DualParaboloidPointLightRenderer _paraboloidPointLR;
	CascadedDirectionalLightRenderer _cascadedDirectionalLR;
	SpotLightRenderer _spotLR;

	HDRPostProcess _hdrPP;
	SkyPostProcess _skyPP;
	MLAAPostProcess _mlaaPP;
	FXAAPostProcess _fxaaPP;
	SSAOPostProcess _ssaoPP;
	HBAOPostProcess _hbaoPP;
	DiscDoFPostProcess _discDoFPP;
	MotionBlurPostProcess _motionBlurPP;
	UIPostProcess _uiPP;

	Texture2DLoader _textureLoader;
	PixelShaderLoader _shaderLoader;

	ConfigurationWindow* _configWindow;
	PostProcessSelectionPane* _ppConfigPane;

	LogWindow* _logWindow;

protected:
	void OnInitialize();
	void OnPreparingContentManager(ContentManager* contentManager);
	void OnPreparingDeviceSettings(DeviceManager* deviceManager);

public:
	DeferredRendererApplication();

	void OnFrameMove(double totalTime, float dt);
	LRESULT OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HRESULT OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice();

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain();
};