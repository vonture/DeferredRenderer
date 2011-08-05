#pragma once

#include "Defines.h"
#include "Gwen/Controls/Base.h"
#include "Gwen/Controls/HorizontalSlider.h"
#include "Gwen/Controls/Label.h"

class SliderWithLabel : public Gwen::Controls::Base
{
private:
	Gwen::Controls::Label* _label;
	Gwen::Controls::HorizontalSlider* _slider;

public:
	SliderWithLabel(Gwen::Controls::Base* parent);

	virtual Gwen::Controls::Label* Label() { return _label; }
	virtual Gwen::Controls::Slider* Slider() { return _slider; }
};