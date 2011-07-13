#pragma once

#include "Gwen\Gwen.h"
#include "IHasContent"

class UIRenderer : public Gwen::Renderer::Base, public IHasContent
{
};