#pragma once

#include "PCH.h"
#include "Application.h"
#include "Renderer.h"
#include "TestingCamera.h"
#include "KeyboardState.h"
#include "MouseState.h"

#include "ParticleCombinePostProcess.h"
#include "HDRPostProcess.h"
#include "SkyPostProcess.h"
#include "MLAAPostProcess.h"
#include "SSAOPostProcess.h"
#include "HBAOPostProcess.h"
#include "DiscDoFMBPostProcess.h"
#include "MotionBlurPostProcess.h"
#include "FilmGrainVignettePostProcess.h"
#include "UIPostProcess.h"
#include "FXAAPostProcess.h"
#include "BoundingObjectPostProcess.h"

#include "DualParaboloidPointLightRenderer.h"
#include "CascadedDirectionalLightRenderer.h"
#include "SpotLightRenderer.h"

#include "PixelShaderLoader.h"
#include "GeometryShaderLoader.h"
#include "VertexShaderLoader.h"
#include "TextureLoader.h"
#include "ModelLoader.h"
#include "ParticleSystemLoader.h"
#include "FontLoader.h"

#include "ConfigurationWindow.h"
#include "PostProcessSelectionPane.h"
#include "ModelConfigurationPane.h"
#include "ParticleConfigurationPane.h"
#include "BoundingObjectConfigurationPane.h"
#include "LogWindow.h"

class DeferredRendererApplication : public Application
{
private:
	Renderer _renderer;
	TestingCamera _camera;

	std::vector<PointLight*> _pointLightsShadowed;
	std::vector<PointLight*> _pointLightsUnshadowed;

	std::vector<DirectionalLight*> _dirLightsShadowed;
	std::vector<DirectionalLight*> _dirLightsUnshadowed;

	std::vector<ModelInstance*> _models;
	std::vector<ParticleSystemInstance*> _particles;

	std::vector<IDragable*> _dragables;

	std::vector<IHasContent*> _contentHolders;

	IDragable* _selectedItem;
	float _selectedDepth;

	DualParaboloidPointLightRenderer _paraboloidPointLR;
	CascadedDirectionalLightRenderer _cascadedDirectionalLR;
	SpotLightRenderer _spotLR;

	ParticleCombinePostProcess _pBufferCombinePP;
	HDRPostProcess _hdrPP;
	SkyPostProcess _skyPP;
	MLAAPostProcess _mlaaPP;
	FXAAPostProcess _fxaaPP;
	SSAOPostProcess _ssaoPP;
	HBAOPostProcess _hbaoPP;
	DiscDoFMBPostProcess _discDoFPP;
	MotionBlurPostProcess _motionBlurPP;
	FilmGrainVignettePostProcess _filmGrainPP;
	BoundingObjectPostProcess _boPP;
	UIPostProcess _uiPP;

	TextureLoader _textureLoader;
	PixelShaderLoader _psLoader;
	GeometryShaderLoader _gsLoader;
	VertexShaderLoader _vsLoader;
	ModelLoader _modelLoader;
	ParticleSystemLoader _particleLoader;
	FontLoader _fontLoader;

	ConfigurationWindow* _configWindow;
	PostProcessSelectionPane* _ppConfigPane;
	ModelConfigurationPane* _modelConfigPane;
	ParticleConfigurationPane* _particleConfigPane;
	BoundingObjectConfigurationPane* _boConfigPane;

	LogWindow* _logWindow;

protected:
	void OnInitialize();
	void OnPreparingContentManager(ContentManager* contentManager);
	void OnPreparingDeviceSettings(DeviceManager* deviceManager);

public:
	DeferredRendererApplication();
	~DeferredRendererApplication();

	void OnFrameMove(double totalTime, float dt);
	LRESULT OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HRESULT OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext);

	HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	void OnD3D11DestroyDevice(ContentManager* pContentManager);

	HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11ReleasingSwapChain(ContentManager* pContentManager);
};