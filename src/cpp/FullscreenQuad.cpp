#include "DXUT.h"
#include "FullscreenQuad.h"

void FullscreenQuad::fillVerticies(QUAD_VERTEX* verts)
{
	verts[0].Position = XMFLOAT4(-1.0f, 1.0f, 0.5f, 1.0f);
    verts[0].TexCoord = XMFLOAT2(0.0f, 0.0f );

    verts[1].Position = XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f);
    verts[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

    verts[2].Position = XMFLOAT4(-1.0f, -1.0f, 0.5f, 1.0f);
    verts[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

    verts[3].Position = XMFLOAT4(1.0f, -1.0f, 0.5f, 1.0f);
    verts[3].TexCoord = XMFLOAT2(1.0f, 1.0f);
}

