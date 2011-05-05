#pragma once

struct ButtonState
{
	bool Pressed;
	bool JustPressed;
	bool JustReleased;

	ButtonState() 
		: Pressed(false), JustPressed(false), JustReleased(false)
	{
	}
};