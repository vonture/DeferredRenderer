#pragma once

#include "Defines.h"

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