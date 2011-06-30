#include "Window.h"

Window::Window(HINSTANCE hinstance, const WCHAR* name, const WCHAR* iconResource, DWORD width,
	DWORD height, DWORD style,	DWORD extendedStyle, const WCHAR* menuResource, const WCHAR* accelResource)
	: _hwnd(NULL), _name(name), _hinstance(hinstance), _style(style), _extendedStyle(extendedStyle),
	  _acceleratorTable(NULL), _maximized(false)
{
	if (!hinstance)
	{
		_hinstance = GetModuleHandle(NULL);
	}

	INITCOMMONCONTROLSEX cce;
	cce.dwSize = sizeof(INITCOMMONCONTROLSEX);
	cce.dwICC = ICC_BAR_CLASSES|ICC_COOL_CLASSES|ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&cce);

	makeWindow(name, iconResource, menuResource);
	SetClientSize(width, height); 

	if (accelResource)
	{
		_acceleratorTable = LoadAccelerators(hinstance, accelResource);
		if (!_acceleratorTable)
		{
			// ERROR
		}
	}
}

Window::~Window()
{
	DestroyWindow(_hwnd);
	UnregisterClass(_name, _hinstance);
}

void Window::makeWindow(const WCHAR* name, const WCHAR* sIconResource, const WCHAR* sMenuResource)
{
	HICON hIcon = NULL;
	if (sIconResource)
	{
		hIcon = (HICON)LoadImage(_hinstance, sIconResource, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
	}
	
	HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);

	// Register the window class
    WNDCLASSEX wc =
	{	
		sizeof(WNDCLASSEX), //UINT        cbSize;
		CS_DBLCLKS,			//UINT        style;
		wndProc,			//WNDPROC     lpfnWndProc;
		0,					//int         cbClsExtra;
		0,					//int         cbWndExtra;
		_hinstance,			//HINSTANCE   hInstance;
		hIcon,				//HICON       hIcon;
		hCursor,			//HCURSOR     hCursor;
		NULL,				//HCURSOR     hCursor;
		sMenuResource,		//LPCWSTR     lpszMenuName;
		name,				//LPCWSTR     lpszClassName;
		NULL,				//HICON       hIconSm;
	};

	if (!RegisterClassEx(&wc))
	{
		// ERROR
		_ASSERT(false);
	}

    // Create the application's window
	_hwnd = CreateWindowEx(	_extendedStyle, name, name, _style, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, _hinstance, (void*)this);

	if (!_hwnd)
	{
		// ERROR
		_ASSERT(false);
	}
}

LRESULT WINAPI Window::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
    {
		case WM_NCCREATE:
		{
			LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT)lParam;
		    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreateStruct->lpCreateParams);
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
    }

    Window* pObj = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (pObj)
	{
		return pObj->MessageHandler(hWnd, uMsg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

LRESULT Window::MessageHandler( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (_messageFunctions.find(uMsg) != _messageFunctions.end())
	{
		MessageFunction* msgFunction = _messageFunctions[uMsg];
		return msgFunction(hWnd, uMsg, wParam, lParam);
	}
	else
	{
		switch (uMsg)
		{
			// Window is being destroyed
			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;

			// Window is being closed
			case WM_CLOSE:
				Destroy();
				return 0;
		}	
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool Window::IsAlive() const
{
	return IsWindow(_hwnd) != 0;
}

bool Window::IsMinimized() const
{
	return IsIconic(_hwnd) != 0;
}

bool Window::IsActive() const
{
	return GetForegroundWindow() == _hwnd;
}

void Window::SetMaximized(bool maximized)
{
	if (maximized != _maximized)
	{
		_maximized = maximized;

		if (_maximized)
		{
			// Create the new style that has no menu bars or borders
			DWORD newStyle = _style & 
				~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
			DWORD newExStyle = _extendedStyle & 
				~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);

			// Save information so that we can be un-maximized
			GetWindowRect(_hwnd, &_unmaxedRect);

			// Determine the monitor we are on
			HMONITOR curMonitor = MonitorFromWindow(_hwnd, MONITOR_DEFAULTTOPRIMARY);

			MONITORINFO monitorInfo;
			monitorInfo.cbSize = sizeof(MONITORINFO);

			GetMonitorInfo(curMonitor, &monitorInfo);

			// Determine the position to put it at
			UINT x = monitorInfo.rcMonitor.left;
			UINT y = monitorInfo.rcMonitor.top;
			UINT width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
			UINT height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

			// place the window
			_ASSERT(SetWindowLong(_hwnd, GWL_STYLE, newStyle));
			_ASSERT(SetWindowLong(_hwnd, GWL_EXSTYLE, newExStyle));
			_ASSERT(SetWindowPos(_hwnd, HWND_TOP, x, y, width, height, SWP_FRAMECHANGED | SWP_SHOWWINDOW));
		}
		else
		{
			// Determine the location to put the window back to
			UINT x = _unmaxedRect.left;
			UINT y = _unmaxedRect.top;
			UINT width = _unmaxedRect.right - _unmaxedRect.left;
			UINT height = _unmaxedRect.bottom - _unmaxedRect.top;

			_ASSERT(SetWindowLong(_hwnd, GWL_STYLE, _style));
			_ASSERT(SetWindowLong(_hwnd, GWL_EXSTYLE, _extendedStyle));
			_ASSERT(SetWindowPos(_hwnd, HWND_NOTOPMOST, x, y, width, height, SWP_FRAMECHANGED | SWP_SHOWWINDOW));
		}
	}
}

bool Window::GetMaximized() const
{
	return _maximized;
}

UINT Window::GetClientWidth() const
{
	RECT clientRect;
	if (!GetClientRect(_hwnd, &clientRect))
	{
		_ASSERT(false);
	}

	return clientRect.right;
}

UINT Window::GetClientHeight() const
{
	RECT clientRect;
	if (!GetClientRect(_hwnd, &clientRect))
	{
		_ASSERT(false);
	}

	return clientRect.bottom;
}

void Window::SetClientSize(UINT width, UINT height)
{
	RECT windowRect;
	SetRect(&windowRect, 0, 0, width, height);

	BOOL bIsMenu = (GetMenu() != NULL);

	if (!AdjustWindowRectEx(&windowRect, _style, bIsMenu, _extendedStyle))
	{
		_ASSERT(false);
	}

	if (!SetWindowPos(_hwnd, HWND_NOTOPMOST, 0, 0, windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top, SWP_NOMOVE))
	{
		_ASSERT(false);
	}
}

UINT Window::GetPositionX() const
{
	RECT windowRect;
	if (!::GetWindowRect(_hwnd, &windowRect))
	{
		_ASSERT(false);
	}

	return windowRect.left;
}

UINT Window::GetPositionY() const
{
	RECT windowRect;
	if (!::GetWindowRect(_hwnd, &windowRect))
	{
		_ASSERT(false);
	}

	return windowRect.top;
}

void Window::SetPosition(UINT x, UINT y)
{
	if (!SetWindowPos(_hwnd, HWND_NOTOPMOST, x, y, 0, 0, SWP_NOSIZE))
	{
		_ASSERT(false);
	}	
}

void Window::SetTitle(const WCHAR* title)
{
	if (!SetWindowText(_hwnd, title))
	{
		_ASSERT(false);
	}
}

void Window::Show()
{
	ShowWindow(_hwnd, SW_SHOW);
}

void Window::Hide()
{
	ShowWindow(_hwnd, SW_HIDE);
}

void Window::Destroy()
{
	DestroyWindow(_hwnd);
	UnregisterClass(_name, _hinstance);
}

void Window::MessageLoop()
{
	// Main message loop:
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		bool isDlgMsg = false;
		
		/*
		for (map<HWND, DlgFunction>::iterator it = userDlgFunctions.begin(); it != userDlgFunctions.end(); it++)
		{
			if (::IsDialogMessage((*it).first, &msg))
			{
				isDlgMsg = true;
				break;
			}
		}
		*/
		
		if (!isDlgMsg)
		{
			if (!_acceleratorTable || !TranslateAccelerator(msg.hwnd, _acceleratorTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
}

void Window::RegisterMessageFunction(UINT message, MessageFunction* function)
{
	_messageFunctions[message] = function;
}