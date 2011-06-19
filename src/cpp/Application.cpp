#include "Application.h"

Application* MainApplication;
LRESULT OnMainApplicationResized(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return MainApplication->OnResized(hWnd, msg, wParam, lParam);
}

Application::Application(const WCHAR* title, const WCHAR* icon)
	: _window(NULL, title, WS_OVERLAPPEDWINDOW, WS_EX_APPWINDOW, 1280, 720, icon)
{
}

Application::~Application()
{
}

LRESULT Application::OnResized(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr;

	if(!_deviceManager.GetFullScreen() && wParam != SIZE_MINIMIZED)
    {
        UINT width = _window.GetClientWidth();
		UINT height = _window.GetClientHeight();
			
		if(width != _deviceManager.GetBackBufferWidth() || 
		   height != _deviceManager.GetBackBufferHeight())
		{
			OnD3D11ReleasingSwapChain();

			_deviceManager.SetBackBufferWidth(width);
            _deviceManager.SetBackBufferHeight(height);
            _deviceManager.Reset();

			V_RETURN(OnD3D11ResizedSwapChain(_deviceManager.GetDevice(), _deviceManager.GetSwapChain(), 
				_deviceManager.GetBackBufferSurfaceDesc()));
		}
	}

	return 0;
}

void Application::OnPreparingDeviceSettings(DeviceManager* deviceManager)
{
}

void Application::OnInitialize()
{
}

HRESULT Application::Start()
{
	HRESULT hr;

	OnInitialize();
		
	// Register this application instance as the main one for callbacks
	MainApplication = this;
	_window.RegisterMessageFunction(WM_SIZE, &OnMainApplicationResized);

	// Ask the application to apply settings and initialize the device
	OnPreparingDeviceSettings(&_deviceManager);
	V_RETURN(_deviceManager.Initialize(_window.GetHWND()));

	// Call the IHasContent methods
	V_RETURN(OnD3D11CreateDevice(_deviceManager.GetDevice(), _deviceManager.GetBackBufferSurfaceDesc()));
	V_RETURN(OnD3D11ResizedSwapChain(_deviceManager.GetDevice(), _deviceManager.GetSwapChain(),
		_deviceManager.GetBackBufferSurfaceDesc()));
		
	// Prepare the counters to use for timing
	LARGE_INTEGER largeInt;
	if (!QueryPerformanceCounter(&largeInt))
	{
		return E_FAIL;
	}
	INT64 startTime = largeInt.QuadPart;
	INT64 minimizedTime = 0;
	INT64 prevTime = startTime;

	if (!QueryPerformanceFrequency(&largeInt))
	{
		return E_FAIL;
	}
	double counterFreq = (double)largeInt.QuadPart;

	// Everything is ready, show the window
	_window.Show();	

	// Begin the main loop
	while(_window.IsAlive())
	{
		// Calculate times even if the window is minimized so that there is not a giant time delta
		// in the next update
		if (!QueryPerformanceCounter(&largeInt))
		{
			return E_FAIL;
		}
		INT64 curTime = largeInt.QuadPart - startTime;

		if (!_window.IsMinimized())
		{
			float deltaSeconds = (float)((curTime - prevTime) / (double)counterFreq);
			double totalSeconds = (curTime - minimizedTime) / (double)counterFreq;

			OnFrameMove(totalSeconds, deltaSeconds);
			V_RETURN(OnD3D11FrameRender(_deviceManager.GetDevice(), _deviceManager.GetImmediateContext()));

			V_RETURN(_deviceManager.Present());
		}
		else
		{
			INT64 delta = curTime - prevTime;
			minimizedTime += delta;
		}

		prevTime = curTime;

		_window.MessageLoop();
	}

	OnD3D11ReleasingSwapChain();
	OnD3D11DestroyDevice();	

	return S_OK;
}

void Application::Exit()
{
	_window.Destroy();
}

HRESULT Application::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void Application::OnD3D11DestroyDevice()
{
}

HRESULT Application::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                            const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	return S_OK;
}

void Application::OnD3D11ReleasingSwapChain()
{
}