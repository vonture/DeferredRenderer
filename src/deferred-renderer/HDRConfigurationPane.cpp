#include "PCH.h"
#include "HDRConfigurationPane.h"

HDRConfigurationPane::HDRConfigurationPane(Gwen::Controls::Base* parent, HDRPostProcess* pp)
	: ConfigurationPane(parent, L"HDR", pp)
{
	_tauSlider = new SliderWithLabel(this);
	_tauSlider->Slider()->SetRange(0.0f, 1.0f);
	_tauSlider->Slider()->SetValue(pp->GetTau());
	_tauSlider->Dock(Gwen::Pos::Top);
	_tauSlider->Slider()->onValueChanged.Add(this, &HDRConfigurationPane::OnValueChanged);
	
	_whiteLumPercSlider = new SliderWithLabel(this);
	_whiteLumPercSlider->Slider()->SetRange(0.0f, 10.0f);
	_whiteLumPercSlider->Slider()->SetValue(pp->GetWhiteLuminancePercentage());
	_whiteLumPercSlider->Dock(Gwen::Pos::Top);
	_whiteLumPercSlider->Slider()->onValueChanged.Add(this, &HDRConfigurationPane::OnValueChanged);

	_bloomThresholdSlider = new SliderWithLabel(this);
	_bloomThresholdSlider->Slider()->SetRange(0.0f, 1.0f);
	_bloomThresholdSlider->Slider()->SetValue(pp->GetBloomThreshold());
	_bloomThresholdSlider->Dock(Gwen::Pos::Top);
	_bloomThresholdSlider->Slider()->onValueChanged.Add(this, &HDRConfigurationPane::OnValueChanged);
	
	_bloomMagnitudeSlider = new SliderWithLabel(this);
	_bloomMagnitudeSlider->Slider()->SetRange(0.0f, 5.0f);
	_bloomMagnitudeSlider->Slider()->SetValue(pp->GetBloomMagnitude());
	_bloomMagnitudeSlider->Dock(Gwen::Pos::Top);
	_bloomMagnitudeSlider->Slider()->onValueChanged.Add(this, &HDRConfigurationPane::OnValueChanged);
	
	_bloomBlurSigmaSlider = new SliderWithLabel(this);
	_bloomBlurSigmaSlider->Slider()->SetRange(0.0f, 4.0f);
	_bloomBlurSigmaSlider->Slider()->SetValue(pp->GetBloomBlurSigma());
	_bloomBlurSigmaSlider->Dock(Gwen::Pos::Top);
	_bloomBlurSigmaSlider->Slider()->onValueChanged.Add(this, &HDRConfigurationPane::OnValueChanged);
	
	_exposureKeySlider = new SliderWithLabel(this);
	_exposureKeySlider->Slider()->SetRange(0.0f, 1.0f);
	_exposureKeySlider->Slider()->SetValue(pp->GetExposureKey());
	_exposureKeySlider->Dock(Gwen::Pos::Top);
	_exposureKeySlider->Slider()->onValueChanged.Add(this, &HDRConfigurationPane::OnValueChanged);
	
	_timeScaleSlider = new SliderWithLabel(this);
	_timeScaleSlider->Slider()->SetRange(0.0f, 10.0f);
	_timeScaleSlider->Slider()->SetValue(1.0f);
	_timeScaleSlider->Dock(Gwen::Pos::Top);
	_timeScaleSlider->Slider()->onValueChanged.Add(this, &HDRConfigurationPane::OnValueChanged);
}

void HDRConfigurationPane::OnValueChanged(Gwen::Controls::Base *control)
{
	HDRPostProcess* pp = GetConfiguredObject();

	if (control == _tauSlider->Slider())
	{
		pp->SetTau(_tauSlider->Slider()->GetValue());
	}
	else if (control == _whiteLumPercSlider->Slider())
	{
		pp->SetWhiteLuminancePercentage(_whiteLumPercSlider->Slider()->GetValue());
	}
	else if (control == _bloomThresholdSlider->Slider())
	{
		pp->SetBloomThreshold(_bloomThresholdSlider->Slider()->GetValue());
	}
	else if (control == _bloomMagnitudeSlider->Slider())
	{
		pp->SetBloomMagnitude(_bloomMagnitudeSlider->Slider()->GetValue());
	}
	else if (control == _bloomBlurSigmaSlider->Slider())
	{
		pp->SetBloomBlurSigma(_bloomBlurSigmaSlider->Slider()->GetValue());
	}
	else if (control == _exposureKeySlider->Slider())
	{
		pp->SetExposureKey(_exposureKeySlider->Slider()->GetValue());
	}
}

void HDRConfigurationPane::OnFrameMove(double totalTime, float dt)
{
	HDRPostProcess* pp = GetConfiguredObject();
	
	_tauSlider->Slider()->SetValue(pp->GetTau());
	_tauSlider->Label()->SetText("Tau: " + Gwen::Utility::ToString(pp->GetTau()));

	_whiteLumPercSlider->Slider()->SetValue(pp->GetWhiteLuminancePercentage());
	_whiteLumPercSlider->Label()->SetText("White luminance percentage: " + Gwen::Utility::ToString(pp->GetWhiteLuminancePercentage()));

	_bloomThresholdSlider->Slider()->SetValue(pp->GetBloomThreshold());
	_bloomThresholdSlider->Label()->SetText("Bloom threshold: " + Gwen::Utility::ToString(pp->GetBloomThreshold()));

	_bloomMagnitudeSlider->Slider()->SetValue(pp->GetBloomMagnitude());
	_bloomMagnitudeSlider->Label()->SetText("Bloom magnitude: " + Gwen::Utility::ToString(pp->GetBloomMagnitude()));
	
	_bloomBlurSigmaSlider->Slider()->SetValue(pp->GetBloomBlurSigma());
	_bloomBlurSigmaSlider->Label()->SetText("Bloom blur sigma: " + Gwen::Utility::ToString(pp->GetBloomBlurSigma()));

	_exposureKeySlider->Slider()->SetValue(pp->GetExposureKey());
	_exposureKeySlider->Label()->SetText("Exposure key: " + Gwen::Utility::ToString(pp->GetExposureKey()));

	float timeScale = _timeScaleSlider->Slider()->GetValue();
	pp->SetTimeDelta(dt * timeScale);
	_timeScaleSlider->Label()->SetText("Time scale: " + Gwen::Utility::ToString(timeScale));
}