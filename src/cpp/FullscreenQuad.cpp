#include "DXUT.h"
#include "FullscreenQuad.h"

void FullscreenQuad::fillVerticies(QUAD_VERTEX* verts)
{
	verts[0].Position = D3DXVECTOR4( -1.0f, 1.0f, 0.5f, 1.0f );
    verts[0].TexCoord = D3DXVECTOR2( 0.0f, 0.0f );

    verts[1].Position = D3DXVECTOR4( 1.0f, 1.0f, 0.5f, 1.0f );
    verts[1].TexCoord = D3DXVECTOR2( 1.0f, 0.0f );

    verts[2].Position = D3DXVECTOR4( -1.0f, -1.0f, 0.5f, 1.0f );
    verts[2].TexCoord = D3DXVECTOR2( 0.0f, 1.0f );

    verts[3].Position = D3DXVECTOR4( 1.0f, -1.0f, 0.5f, 1.0f );
    verts[3].TexCoord = D3DXVECTOR2( 1.0f, 1.0f );
}