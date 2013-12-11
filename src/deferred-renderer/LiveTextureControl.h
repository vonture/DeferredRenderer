#pragma once

#include "PCH.h"
#include "Gwen/Controls/Base.h"
#include "Gwen/Texture.h"

class LiveTextureControl : public Gwen::Controls::Base
{
private:
    Gwen::Texture _tex;

protected:
    virtual void Render(Gwen::Skin::Base* skin);

public:
    LiveTextureControl(Gwen::Controls::Base* parent);

    void SetTexture(ID3D11ShaderResourceView* srv);
};