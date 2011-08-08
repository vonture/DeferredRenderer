#include "PCH.h"
#include "PoissonDoFConfigurationPane.h"

PoissonDoFConfigurationPane::PoissonDoFConfigurationPane(Gwen::Controls::Base* parent, 
	PoissonDoFPostProcess* pp)
	: ConfigurationPane(parent, L"Poisson DoF", pp)
{
	_focalDistSlider = new SliderWithLabel(this);
	_focalDistSlider->Slider()->SetRange(0.0f, 100.0f);
	_focalDistSlider->Slider()->SetValue(pp->GetFocalDistance());
	_focalDistSlider->Slider()->onValueChanged.Add(this, &PoissonDoFConfigurationPane::OnValueChanged);
	_focalDistSlider->Dock(Gwen::Pos::Top);

	_falloffStartSlider = new SliderWithLabel(this);
	_falloffStartSlider->Slider()->SetRange(0.0f, 100.0f);
	_falloffStartSlider->Slider()->SetValue(pp->GetFocalFalloffStart());
	_falloffStartSlider->Slider()->onValueChanged.Add(this, &PoissonDoFConfigurationPane::OnValueChanged);
	_falloffStartSlider->Dock(Gwen::Pos::Top);

	_falloffEndSlider = new SliderWithLabel(this);
	_falloffEndSlider->Slider()->SetRange(0.0f, 100.0f);
	_falloffEndSlider->Slider()->SetValue(pp->GetFocalFalloffEnd());
	_falloffEndSlider->Slider()->onValueChanged.Add(this, &PoissonDoFConfigurationPane::OnValueChanged);
	_falloffEndSlider->Dock(Gwen::Pos::Top);

	_cocScaleSlider = new SliderWithLabel(this);
	_cocScaleSlider->Slider()->SetRange(0.0f, 15.0f);
	_cocScaleSlider->Slider()->SetValue(pp->GetCircleOfConfusionScale());
	_cocScaleSlider->Slider()->onValueChanged.Add(this, &PoissonDoFConfigurationPane::OnValueChanged);
	_cocScaleSlider->Dock(Gwen::Pos::Top);

	_sampleCountSlider = new SliderWithLabel(this);
	_sampleCountSlider->Slider()->SetClampToNotches(true);
	_sampleCountSlider->Slider()->SetRange(0.0f, pp->GetNumSampleCountIndicies() - 1);
	_sampleCountSlider->Slider()->SetValue(pp->GetSampleCountIndex());	
	_sampleCountSlider->Slider()->onValueChanged.Add(this, &PoissonDoFConfigurationPane::OnValueChanged);
	_sampleCountSlider->Dock(Gwen::Pos::Top);
}

void PoissonDoFConfigurationPane::OnValueChanged(Gwen::Controls::Base *control)
{
	PoissonDoFPostProcess* pp = GetConfiguredObject();

	if (control == _focalDistSlider->Slider())
	{
		pp->SetFocalDistance(_focalDistSlider->Slider()->GetValue());
	}
	else if (control == _falloffStartSlider->Slider() || control == _falloffEndSlider->Slider())
	{
		pp->SetFocalFalloffs(_falloffStartSlider->Slider()->GetValue(), _falloffEndSlider->Slider()->GetValue());
	}
	else if (control == _cocScaleSlider->Slider())
	{
		pp->SetCircleOfConfusionScale(_cocScaleSlider->Slider()->GetValue());
	}
	else if (control == _sampleCountSlider->Slider())
	{
		pp->SetSampleCountIndex(floor(_sampleCountSlider->Slider()->GetValue() + 0.5f));
	}
}

void PoissonDoFConfigurationPane::OnFrameMove(double totalTime, float dt)
{
	PoissonDoFPostProcess* pp = GetConfiguredObject();

	_focalDistSlider->Slider()->SetValue(pp->GetFocalDistance());
	_focalDistSlider->Label()->SetText("Focal distance: " + Gwen::Utility::ToString(pp->GetFocalDistance())); 

	_falloffStartSlider->Slider()->SetValue(pp->GetFocalFalloffStart());
	_falloffStartSlider->Label()->SetText("Falloff start: " + Gwen::Utility::ToString(pp->GetFocalFalloffStart())); 

	_falloffEndSlider->Slider()->SetValue(pp->GetFocalFalloffEnd());
	_falloffEndSlider->Label()->SetText("Falloff end: " + Gwen::Utility::ToString(pp->GetFocalFalloffEnd())); 

	_cocScaleSlider->Slider()->SetValue(pp->GetCircleOfConfusionScale());
	_cocScaleSlider->Label()->SetText("Circle of confusion size: " + Gwen::Utility::ToString(pp->GetCircleOfConfusionScale())); 

	_sampleCountSlider->Slider()->SetValue(pp->GetSampleCountIndex());
	_sampleCountSlider->Label()->SetText("Sample count: " + Gwen::Utility::ToString(pp->GetSampleCount()));
}