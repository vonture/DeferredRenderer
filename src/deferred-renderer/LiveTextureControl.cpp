#include "PCH.h"
#include "LiveTextureControl.h"

LiveTextureControl::LiveTextureControl(Gwen::Controls::Base* parent)
    : Gwen::Controls::Base(parent)
{
    _tex.height = 1;
    _tex.width = 1;
    _tex.failed = false;
    _tex.data = NULL;
    _tex.name = Gwen::TextObject("Live Texture");
}

void LiveTextureControl::Render(Gwen::Skin::Base* skin)
{
    Gwen::Rect drawRect = GetRenderBounds();
    if (_tex.data)
    {
        skin->GetRender()->DrawTexturedRect(&_tex, drawRect);
    }
    else
    {
        skin->GetRender()->DrawMissingImage(drawRect);
    }
}

void LiveTextureControl::SetTexture(ID3D11ShaderResourceView* srv)
{
    _tex.data = srv;
}