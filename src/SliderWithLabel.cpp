#include "PCH.h"
#include "SliderWithLabel.h"

SliderWithLabel::SliderWithLabel(Gwen::Controls::Base* parent)
	: Gwen::Controls::Base(parent)
{
	SetSize(200, 34);

	_label = new Gwen::Controls::Label(this);
	_label->Dock(Gwen::Pos::Top);
	_label->SetTabable(false);

	_slider = new Gwen::Controls::HorizontalSlider(this);
	_slider->Dock(Gwen::Pos::Fill);
	_slider->SetTabable(false);

	SetTabable(false);
}