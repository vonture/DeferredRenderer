#include "PCH.h"
#include "MouseState.h"

MouseState MouseState::_prevState;

MouseState::MouseState() 
	: _x(0), _y(0), _dx(0), _dy(0)
{
}

MouseState::~MouseState()
{
}

int MouseState::GetX()
{
	return _x;
}
	
int MouseState::GetY()
{
	return _y;
}

int MouseState::GetDX()
{
	return _dx;
}

int MouseState::GetDY()
{
	return _dy;
}

bool MouseState::IsOverWindow()
{
	return _overWindow;
}

bool MouseState::IsJustOverWindow()
{
	return _overWindow && !_prevState._overWindow;
}

ButtonState MouseState::GetButtonState(UINT button)
{
	return _buttonStates[button];
}

bool MouseState::IsButtonDown(UINT button)
{
	return _buttonStates[button].Pressed;
}

bool MouseState::IsButtonUp(UINT button)
{
	return !_buttonStates[button].Pressed;
}

bool MouseState::IsButtonJustPressed(UINT button)
{
	return _buttonStates[button].JustPressed;
}

bool MouseState::IsButtonJustReleased(UINT button)
{
	return _buttonStates[button].JustReleased;
}

void MouseState::SetCursorPosition(int x, int y, HWND hwnd)
{
	HRESULT hr;

    POINT pos;
    pos.x = x;
    pos.y = y;

	if (hwnd)
	{
		V_WIN(ClientToScreen(hwnd, &pos));
	}

    V_WIN(SetCursorPos(pos.x, pos.y));

	_prevState._x = x;
	_prevState._y = y;
}

void MouseState::SetCursorVisible(bool visible)
{
	HRESULT hr;

	V_WIN(ShowCursor(visible));
}

MouseState MouseState::GetState(HWND hwnd)
{
	HRESULT hr;

	POINT pos;	
	V_WIN(GetCursorPos(&pos));
	
	// If a window was supplied, transform the point to window space
	if (hwnd)
	{
		V_WIN(ScreenToClient(hwnd, &pos));
	}

	MouseState newState;
	newState._x = pos.x;
	newState._y = pos.y;
	newState._dx = pos.x - _prevState._x;
	newState._dy = pos.y - _prevState._y;

	newState._buttonStates[MouseButton::LeftButton].Pressed = (GetKeyState(VK_LBUTTON) & KF_UP) != 0;
	newState._buttonStates[MouseButton::LeftButton].JustPressed = 
		newState._buttonStates[MouseButton::LeftButton].Pressed && !_prevState._buttonStates[MouseButton::LeftButton].Pressed;
	newState._buttonStates[MouseButton::LeftButton].JustReleased = 
		!newState._buttonStates[MouseButton::LeftButton].Pressed && _prevState._buttonStates[MouseButton::LeftButton].Pressed;

	newState._buttonStates[MouseButton::RightButton].Pressed = (GetKeyState(VK_RBUTTON) & KF_UP) != 0;
	newState._buttonStates[MouseButton::RightButton].JustPressed = 
		newState._buttonStates[MouseButton::RightButton].Pressed && !_prevState._buttonStates[MouseButton::RightButton].Pressed;
	newState._buttonStates[MouseButton::RightButton].JustReleased = 
		!newState._buttonStates[MouseButton::RightButton].Pressed && _prevState._buttonStates[MouseButton::RightButton].Pressed;

	newState._buttonStates[MouseButton::CenterButton].Pressed = (GetKeyState(VK_MBUTTON) & KF_UP) != 0;
	newState._buttonStates[MouseButton::CenterButton].JustPressed = 
		newState._buttonStates[MouseButton::CenterButton].Pressed && !_prevState._buttonStates[MouseButton::CenterButton].Pressed;
	newState._buttonStates[MouseButton::CenterButton].JustReleased = 
		!newState._buttonStates[MouseButton::CenterButton].Pressed && _prevState._buttonStates[MouseButton::CenterButton].Pressed;

	newState._buttonStates[MouseButton::Mouse4].Pressed = (GetKeyState(VK_XBUTTON1) & KF_UP) != 0;
	newState._buttonStates[MouseButton::Mouse4].JustPressed = 
		newState._buttonStates[MouseButton::Mouse4].Pressed && !_prevState._buttonStates[MouseButton::Mouse4].Pressed;
	newState._buttonStates[MouseButton::Mouse4].JustReleased = 
		!newState._buttonStates[MouseButton::Mouse4].Pressed && _prevState._buttonStates[MouseButton::Mouse4].Pressed;

	newState._buttonStates[MouseButton::Mouse5].Pressed = (GetKeyState(VK_XBUTTON2) & KF_UP) != 0;
	newState._buttonStates[MouseButton::Mouse5].JustPressed = 
		newState._buttonStates[MouseButton::Mouse5].Pressed && !_prevState._buttonStates[MouseButton::Mouse5].Pressed;
	newState._buttonStates[MouseButton::Mouse5].JustReleased = 
		!newState._buttonStates[MouseButton::Mouse5].Pressed && _prevState._buttonStates[MouseButton::Mouse5].Pressed;

	if (hwnd)
	{
		RECT clientArea;
		V_WIN(GetClientRect(hwnd, &clientArea));

		newState._overWindow = (pos.x >= 0 && pos.x < clientArea.right && pos.y >= 0 && pos.y < clientArea.bottom);
	}
	else 
	{
		newState._overWindow = false;
	}

	_prevState = newState;
	return _prevState;
}

