#pragma once

#include "Defines.h"
#include "IUpdateable.h"
#include "IHasContent.h"
#include "Window.h"
#include "DeviceManager.h"

class Application : public IUpdateable, public IHasContent
{
private:
	Window _window;
	DeviceManager _deviceManager;
	
protected:
	virtual void OnPreparingDeviceSettings(DeviceManager* deviceManager);
	virtual void OnInitialize();

public:
	Application(const WCHAR* title, const WCHAR* icon);
	~Application();
	
	HRESULT Start();
	void Exit();

	LRESULT OnResized(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	virtual void OnFrameMove(double totalTime, float dt) = 0;
	virtual HRESULT OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext) = 0;

	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);	
	virtual void OnD3D11DestroyDevice();

	virtual HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	virtual void OnD3D11ReleasingSwapChain();
};