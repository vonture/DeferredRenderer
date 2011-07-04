#pragma once

#include "Defines.h"
#include "IHasContent.h"
#include "Renderer.h"

class IDrawable : public IHasContent
{
public:
	virtual HRESULT OnFrameRender(Renderer* renderer, double totalTime, float dt) = 0;
};