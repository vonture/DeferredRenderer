#pragma once

#include "Defines.h"
#include <map>

typedef LRESULT MessageFunction(HWND, UINT, WPARAM, LPARAM);

class Window
{
private:
	const WCHAR* _name;
	HWND _hwnd;
	HINSTANCE _hinstance;
	DWORD _style;
	DWORD _extendedStyle;
	HACCEL _acceleratorTable;

	std::map<UINT, MessageFunction*> _messageFunctions;

	void makeWindow(const WCHAR* name, const WCHAR* sIconResource, const WCHAR* sMenuResource);
	static LRESULT WINAPI wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	Window(HINSTANCE hinstance, const WCHAR* name, DWORD style = WS_CAPTION|WS_OVERLAPPED|WS_SYSMENU,
		DWORD extendedStyle = WS_EX_APPWINDOW, DWORD width = 1280, DWORD height = 720, 
		LPCWSTR iconResource = NULL, LPCWSTR menuResource = NULL, LPCWSTR accelResource = NULL);
	~Window();

	HWND GetHWND() const { return _hwnd; }
	HMENU GetMenu() const { return ::GetMenu(_hwnd); }
	HINSTANCE GetHinstance () const { return _hinstance; }
		
	bool IsAlive() const;
	bool IsMinimized() const;

	UINT GetClientWidth() const;
	UINT GetClientHeight() const;
	void SetClientSize(UINT width, UINT height);

	UINT GetPositionX() const;
	UINT GetPositionY() const;
	void SetPosition(UINT x, UINT y);
	
	void SetTitle(const WCHAR* title);

	void Show();
	void Hide();
	void Destroy();

	void MessageLoop();

	void RegisterMessageFunction(UINT message, MessageFunction* function);
};