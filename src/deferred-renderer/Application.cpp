#include "PCH.h"
#include "Application.h"

using std::tr1::bind;
using std::tr1::mem_fn;
using namespace std::tr1::placeholders;

Application::Application(const WCHAR* title, const WCHAR* icon)
    : _window(NULL, title, icon, 1360, 768)
{
}

Application::~Application()
{
}

LRESULT Application::OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr;

    switch(msg)
    {
    case WM_SIZE:
        {
            if(!_deviceManager.GetFullScreen() && wParam != SIZE_MINIMIZED)
            {
                UINT width = _window.GetClientWidth();
                UINT height = _window.GetClientHeight();

                if(width != _deviceManager.GetBackBufferWidth() || height != _deviceManager.GetBackBufferHeight())
                {
                    OnD3D11ReleasingSwapChain(&_contentManager);

                    _deviceManager.SetBackBufferWidth(width);
                    _deviceManager.SetBackBufferHeight(height);
                    _deviceManager.Reset();

                    V_RETURN(OnD3D11ResizedSwapChain(_deviceManager.GetDevice(), &_contentManager,
                        _deviceManager.GetSwapChain(), _deviceManager.GetBackBufferSurfaceDesc()));
                }
            }
        }
    }

    return 0;
}

void Application::OnPreparingContentManager(ContentManager* contentManager)
{
}

void Application::OnPreparingDeviceSettings(DeviceManager* deviceManager)
{
}

void Application::OnInitialize()
{
}

Window* Application::GetWindow()
{
    return &_window;
}

DeviceManager* Application::GetDeviceManager()
{
    return &_deviceManager;
}

HRESULT Application::Start()
{
    HRESULT hr;

    OnInitialize();

    // Register this application instance as the main one for callbacks
    Window::MessageFunction fn = bind(mem_fn(&Application::OnMessage), this, _1, _2, _3, _4);
    _window.RegisterMessageFunction(fn);

    // Ask the application to apply settings and initialize the device
    OnPreparingDeviceSettings(&_deviceManager);
    V_RETURN(_deviceManager.Initialize(_window.GetHWND()));

    // Prepare content manager
    OnPreparingContentManager(&_contentManager);

    // Set the window to be the same size as the back buffer
    _window.SetClientSize(_deviceManager.GetBackBufferWidth(), _deviceManager.GetBackBufferHeight());

    // Window prepared, show it
    _window.Show();

    // Call the IHasContent methods
    V_RETURN(OnD3D11CreateDevice(_deviceManager.GetDevice(), &_contentManager, _deviceManager.GetBackBufferSurfaceDesc()));
    V_RETURN(OnD3D11ResizedSwapChain(_deviceManager.GetDevice(), &_contentManager, _deviceManager.GetSwapChain(),
        _deviceManager.GetBackBufferSurfaceDesc()));

    // Prepare the counters to use for timing
    LARGE_INTEGER largeInt;
    if (!QueryPerformanceCounter(&largeInt))
    {
        return E_FAIL;
    }
    INT64 startTime = largeInt.QuadPart;
    INT64 prevTime = 0;
    INT64 inactiveTime = 0;

    if (!QueryPerformanceFrequency(&largeInt))
    {
        return E_FAIL;
    }
    double counterFreq = (double)largeInt.QuadPart;

    // Begin the main loop
    while(_window.IsAlive())
    {
        BEGIN_EVENT(L"Main loop");

        // Calculate times even if the window is minimized so that there is not a giant time delta
        // in the next update
        if (!QueryPerformanceCounter(&largeInt))
        {
            return E_FAIL;
        }
        INT64 curTime = largeInt.QuadPart - startTime;

        if (_window.IsActive())
        {
            float deltaSeconds = (float)((curTime - prevTime) / (double)counterFreq);
            double totalSeconds = (curTime - inactiveTime) / (double)counterFreq;

            BEGIN_EVENT(L"Frame move");
            OnFrameMove(totalSeconds, deltaSeconds);
            END_EVENT(L"");

            BEGIN_EVENT(L"Render");
            V_RETURN(OnD3D11FrameRender(_deviceManager.GetDevice(), _deviceManager.GetImmediateContext()));
            END_EVENT(L"");

            BEGIN_EVENT(L"Present");
            V_RETURN(_deviceManager.Present());
            END_EVENT(L"");
        }
        else
        {
            INT64 delta = curTime - prevTime;
            inactiveTime += delta;
        }

        prevTime = curTime;

        _window.MessageLoop();

        END_EVENT(L"");
    }

    // Clean up
    OnD3D11ReleasingSwapChain(&_contentManager);
    OnD3D11DestroyDevice(&_contentManager);

    _deviceManager.Destroy();

    return S_OK;
}

void Application::Exit()
{
    _window.Destroy();
}

void Application::SetFullScreen(bool fullScreen)
{
    HRESULT hr;

    if (fullScreen != _deviceManager.GetFullScreen())
    {
        OnD3D11ReleasingSwapChain(&_contentManager);

        _deviceManager.SetFullScreen(fullScreen);
        _deviceManager.Reset();

        V(OnD3D11ResizedSwapChain(_deviceManager.GetDevice(), &_contentManager, _deviceManager.GetSwapChain(),
            _deviceManager.GetBackBufferSurfaceDesc()));
    }
}

bool Application::GetFullScreen() const
{
    return _deviceManager.GetFullScreen();
}

void Application::SetMaximized(bool maximized)
{
    _window.SetMaximized(maximized);
}

bool Application::GetMaximized() const
{
    return _window.GetMaximized();
}

HWND Application::GetHWND() const
{
    return _window.GetHWND();
}

bool Application::IsActive() const
{
    return _window.IsActive();
}

UINT Application::GetWidth() const
{
    return _deviceManager.GetBackBufferWidth();
}

UINT Application::GetHeight() const
{
    return _deviceManager.GetBackBufferHeight();
}

HRESULT Application::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, ContentManager* pContentManager, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    return S_OK;
}

void Application::OnD3D11DestroyDevice(ContentManager* pContentManager)
{
}

HRESULT Application::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, ContentManager* pContentManager, IDXGISwapChain* pSwapChain,
                                             const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
    return S_OK;
}

void Application::OnD3D11ReleasingSwapChain(ContentManager* pContentManager)
{
}