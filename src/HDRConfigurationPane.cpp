#include "HDRConfigurationPane.h"

HDRConfigurationPane::HDRConfigurationPane(Gwen::Controls::Base* parent, HDRPostProcess* pp)
	: ConfigurationPane(parent, pp), _tauSlider(NULL), _whiteLumPercSlider(NULL), _bloomThresholdSlider(NULL),
	  _bloomMagnitudeSlider(NULL), _bloomBlurSigmaSlider(NULL), _timeScaleSlider(NULL), _tauLabel(NULL),
	  _whiteLumPercLabel(NULL), _bloomThresholdLabel(NULL), _bloomMagnitudeLabel(NULL), _bloomBlurSigmaLabel(NULL),
	  _exposureKeyLabel(NULL), _timeScaleLabel(NULL)
{
	SetName("HDR Post Process");

	_tauLabel = new Gwen::Controls::Label(this);
	_tauLabel->SetHeight(16);
	_tauLabel->SetText("");
	_tauLabel->Dock(Gwen::Pos::Top);

	_tauSlider = new Gwen::Controls::HorizontalSlider(this);
	_tauSlider->SetClampToNotches(false);
	_tauSlider->SetRange(0.0f, 1.0f);
	_tauSlider->SetValue(pp->GetTau());
	_tauSlider->SetHeight(16);
	_tauSlider->Dock(Gwen::Pos::Top);	

	_whiteLumPercLabel = new Gwen::Controls::Label(this);
	_whiteLumPercLabel->SetHeight(16);
	_whiteLumPercLabel->SetText("");
	_whiteLumPercLabel->Dock(Gwen::Pos::Top);

	_whiteLumPercSlider = new Gwen::Controls::HorizontalSlider(this);
	_whiteLumPercSlider->SetClampToNotches(false);
	_whiteLumPercSlider->SetRange(0.0f, 10.0f);
	_whiteLumPercSlider->SetValue(pp->GetWhiteLuminancePercentage());
	_whiteLumPercSlider->SetHeight(16);
	_whiteLumPercSlider->Dock(Gwen::Pos::Top);

	_bloomThresholdLabel = new Gwen::Controls::Label(this);
	_bloomThresholdLabel->SetHeight(16);
	_bloomThresholdLabel->SetText("");
	_bloomThresholdLabel->Dock(Gwen::Pos::Top);

	_bloomThresholdSlider = new Gwen::Controls::HorizontalSlider(this);
	_bloomThresholdSlider->SetClampToNotches(false);
	_bloomThresholdSlider->SetRange(0.0f, 1.0f);
	_bloomThresholdSlider->SetValue(pp->GetBloomThreshold());
	_bloomThresholdSlider->SetHeight(16);
	_bloomThresholdSlider->Dock(Gwen::Pos::Top);

	_bloomMagnitudeLabel = new Gwen::Controls::Label(this);
	_bloomMagnitudeLabel->SetHeight(16);
	_bloomMagnitudeLabel->SetText("");
	_bloomMagnitudeLabel->Dock(Gwen::Pos::Top);

	_bloomMagnitudeSlider = new Gwen::Controls::HorizontalSlider(this);
	_bloomMagnitudeSlider->SetClampToNotches(false);
	_bloomMagnitudeSlider->SetRange(0.0f, 5.0f);
	_bloomMagnitudeSlider->SetValue(pp->GetBloomMagnitude());
	_bloomMagnitudeSlider->SetHeight(16);
	_bloomMagnitudeSlider->Dock(Gwen::Pos::Top);

	_bloomBlurSigmaLabel = new Gwen::Controls::Label(this);
	_bloomBlurSigmaLabel->SetHeight(16);
	_bloomBlurSigmaLabel->SetText("");
	_bloomBlurSigmaLabel->Dock(Gwen::Pos::Top);

	_bloomBlurSigmaSlider = new Gwen::Controls::HorizontalSlider(this);
	_bloomBlurSigmaSlider->SetClampToNotches(false);
	_bloomBlurSigmaSlider->SetRange(0.0f, 4.0f);
	_bloomBlurSigmaSlider->SetValue(pp->GetBloomBlurSigma());
	_bloomBlurSigmaSlider->SetHeight(16);
	_bloomBlurSigmaSlider->Dock(Gwen::Pos::Top);

	_exposureKeyLabel = new Gwen::Controls::Label(this);
	_exposureKeyLabel->SetHeight(16);
	_exposureKeyLabel->SetText("");
	_exposureKeyLabel->Dock(Gwen::Pos::Top);

	_exposureKeySlider = new Gwen::Controls::HorizontalSlider(this);
	_exposureKeySlider->SetClampToNotches(false);
	_exposureKeySlider->SetRange(0.0f, 1.0f);
	_exposureKeySlider->SetValue(pp->GetExposureKey());
	_exposureKeySlider->SetHeight(16);
	_exposureKeySlider->Dock(Gwen::Pos::Top);

	_timeScaleLabel = new Gwen::Controls::Label(this);
	_timeScaleLabel->SetHeight(16);
	_timeScaleLabel->SetText("");
	_timeScaleLabel->Dock(Gwen::Pos::Top);

	_timeScaleSlider = new Gwen::Controls::HorizontalSlider(this);
	_timeScaleSlider->SetClampToNotches(false);
	_timeScaleSlider->SetRange(0.0f, 10.0f);
	_timeScaleSlider->SetValue(1.0f);
	_timeScaleSlider->SetHeight(16);
	_timeScaleSlider->Dock(Gwen::Pos::Top);
}

HDRConfigurationPane::~HDRConfigurationPane()
{
}

void HDRConfigurationPane::OnFrameMove(double totalTime, float dt)
{
	HDRPostProcess* pp = GetConfiguredObject();
	
	pp->SetTau(_tauSlider->GetValue());
	_tauSlider->SetValue(pp->GetTau());
	_tauLabel->SetText("Tau: " + Gwen::Utility::ToString(pp->GetTau()));

	pp->SetWhiteLuminancePercentage(_whiteLumPercSlider->GetValue());
	_whiteLumPercSlider->SetValue(pp->GetWhiteLuminancePercentage());
	_whiteLumPercLabel->SetText("White luminance percentage: " + Gwen::Utility::ToString(pp->GetWhiteLuminancePercentage()));

	pp->SetBloomThreshold(_bloomThresholdSlider->GetValue());
	_bloomThresholdSlider->SetValue(pp->GetBloomThreshold());
	_bloomThresholdLabel->SetText("Bloom threshold: " + Gwen::Utility::ToString(pp->GetBloomThreshold()));

	pp->SetBloomMagnitude(_bloomMagnitudeSlider->GetValue());
	_bloomMagnitudeSlider->SetValue(pp->GetBloomMagnitude());
	_bloomMagnitudeLabel->SetText("Bloom magnitude: " + Gwen::Utility::ToString(pp->GetBloomMagnitude()));
	
	pp->SetBloomBlurSigma(_bloomBlurSigmaSlider->GetValue());
	_bloomBlurSigmaSlider->SetValue(pp->GetBloomBlurSigma());
	_bloomBlurSigmaLabel->SetText("Bloom blur sigma: " + Gwen::Utility::ToString(pp->GetBloomBlurSigma()));

	pp->SetExposureKey(_exposureKeySlider->GetValue());
	_exposureKeySlider->SetValue(pp->GetExposureKey());
	_exposureKeyLabel->SetText("Exposure key: " + Gwen::Utility::ToString(pp->GetExposureKey()));

	pp->SetTimeDelta(dt * _timeScaleSlider->GetValue());
	_timeScaleLabel->SetText("Time scale: " + Gwen::Utility::ToString(_timeScaleSlider->GetValue()));
}