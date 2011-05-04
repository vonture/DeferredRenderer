#pragma once

#include "DXUT.h"
#include "Quad.h"

class FullscreenQuad : public Quad
{
protected:
	void fillVerticies(QUAD_VERTEX* verts);
};