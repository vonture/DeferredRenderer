#pragma once

#include "Defines.h"

class IUpdateable
{
public:
	virtual void OnFrameMove(double totalTime, float dt) = 0;
};