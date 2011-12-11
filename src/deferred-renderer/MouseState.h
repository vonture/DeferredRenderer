#pragma once

#include "PCH.h"
#include "ButtonState.h"

namespace MouseButton
{
	enum
	{
		LeftButton = 0,
		RightButton = 1,
		CenterButton = 2,
		Mouse4 = 3,
		Mouse5 = 4,
	};
}


class MouseState
{
private:
	int _x, _y, _dx, _dy;
	int _scroll, _dscroll;
	bool _overWindow;
	ButtonState _buttonStates[5];

	static MouseState _prevState;

public:
	MouseState();
	~MouseState();

	static void SetCursorPosition(int x, int y, HWND hwnd = NULL);
	static void SetCursorVisible(bool visible);

	int GetX();
	int GetY();
	int GetDX();
	int GetDY();

	int GetScroll();
	int GetDScroll();

	bool IsOverWindow();
	bool IsJustOverWindow();

	ButtonState GetButtonState(UINT button);
    bool IsButtonDown(UINT button);
	bool IsButtonUp(UINT button);
    bool IsButtonJustPressed(UINT button);
	bool IsButtonJustReleased(UINT button);
	
	static MouseState GetState(HWND hwnd = NULL);
};