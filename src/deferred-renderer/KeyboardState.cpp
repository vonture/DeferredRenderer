#include "PCH.h"
#include "KeyboardState.h"

BYTE KeyboardState::_curState[256];
KeyboardState KeyboardState::_prevState;

ButtonState KeyboardState::GetKeyState(UINT key)
{
    return _keyStates[key];
}

bool KeyboardState::IsKeyDown(UINT key)
{
    return _keyStates[key].Pressed;
}

bool KeyboardState::IsKeyUp(UINT key)
{
    return !_keyStates[key].Pressed;
}

bool KeyboardState::IsKeyJustPressed(UINT key)
{
    return _keyStates[key].JustPressed;
}

bool KeyboardState::IsKeyJustReleased(UINT key)
{
    return _keyStates[key].JustReleased;
}

KeyboardState KeyboardState::GetState()
{
    GetKeyboardState(_curState);

    ButtonState state;
    for (int i = 0; i < 256; i++)
    {
        state.Pressed = keyPressed(_curState[i]);
        state.JustPressed = state.Pressed && !_prevState._keyStates[i].Pressed;
        state.JustReleased = !state.Pressed && _prevState._keyStates[i].Pressed;
        _prevState._keyStates[i] = state;
    }

    return _prevState;
}