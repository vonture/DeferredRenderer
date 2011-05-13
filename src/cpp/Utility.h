#pragma once

const float Pi = 3.14159f;
const float PiOver2 = Pi / 2.0f;
const float PiOver4 = Pi / 4.0f;
const float PiOver8 = Pi / 8.0f;

inline float clampf(float val, float min, float max)
{
	return (val < min) ? min : ((val > max) ? max : val);
}