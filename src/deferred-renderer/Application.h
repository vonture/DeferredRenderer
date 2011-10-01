#pragma once

#include "PCH.h"
#include "IUpdateable.h"
#include "IHasContent.h"
#include "Window.h"
#include "DeviceManager.h"
#include "Logger.h"
#include "ContentManager.h"

class Application : public IUpdateable, public IHasContent
{
private:
	Window _window;
	DeviceManager _deviceManager;
	ContentManager _contentManager;
	
protected:
	virtual void OnPreparingContentManager(ContentManager* contentManager);
	virtual void OnPreparingDeviceSettings(DeviceManager* deviceManager);
	virtual void OnInitialize();

	Window* GetWindow();
	DeviceManager* GetDeviceManager();

public:
	Application(const WCHAR* title, const WCHAR* icon);
	virtual ~Application();

	HRESULT Start();
	void Exit();

	void SetFullScreen(bool fullScreen);
	bool GetFullScreen() const;

	void SetMaximized(bool maximized);
	bool GetMaximized() const;

	bool IsActive() const;

	HWND GetHWND() const;

	virtual LRESULT OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	virtual void OnFrameMove(double totalTime, float dt) = 0;
	virtual HRESULT OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext) = 0;

	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	virtual void OnD3D11DestroyDevice();

	virtual HRESULT OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	virtual void OnD3D11ReleasingSwapChain();
};