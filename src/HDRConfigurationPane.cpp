#include "PCH.h"
#include "HDRConfigurationPane.h"

HDRConfigurationPane::HDRConfigurationPane(Gwen::Controls::Base* parent, HDRPostProcess* pp)
	: ConfigurationPane(parent, L"HDR", pp), _tauSlider(NULL), _whiteLumPercSlider(NULL), _bloomThresholdSlider(NULL),
	  _bloomMagnitudeSlider(NULL), _bloomBlurSigmaSlider(NULL), _timeScaleSlider(NULL), _tauLabel(NULL),
	  _whiteLumPercLabel(NULL), _bloomThresholdLabel(NULL), _bloomMagnitudeLabel(NULL), _bloomBlurSigmaLabel(NULL),
	  _exposureKeyLabel(NULL), _timeScaleLabel(NULL), _timeScale(1.0f)
{	
	const int childWidth = 240;
	const int labelHight = 20;
	const int sliderHeight = 14;
	const int spacing = 3;

	_tauLabel = new Gwen::Controls::Label(this);
	_tauLabel->SetText("");
	_tauLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_tauLabel->SetBounds(0, 0, childWidth, labelHight);

	_tauSlider = new Gwen::Controls::HorizontalSlider(this);
	_tauSlider->SetClampToNotches(false);
	_tauSlider->SetRange(0.0f, 1.0f);
	_tauSlider->SetValue(pp->GetTau());
	_tauSlider->SetBounds(0, _tauLabel->Bottom(), childWidth, sliderHeight);
	_tauSlider->onValueChanged.Add(this, &HDRConfigurationPane::OnValueChanged);

	_whiteLumPercLabel = new Gwen::Controls::Label(this);
	_whiteLumPercLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_whiteLumPercLabel->SetBounds(0, _tauSlider->Bottom() + spacing, childWidth, labelHight);

	_whiteLumPercSlider = new Gwen::Controls::HorizontalSlider(this);
	_whiteLumPercSlider->SetClampToNotches(false);
	_whiteLumPercSlider->SetRange(0.0f, 10.0f);
	_whiteLumPercSlider->SetValue(pp->GetWhiteLuminancePercentage());
	_whiteLumPercSlider->SetBounds(0, _whiteLumPercLabel->Bottom(), childWidth, sliderHeight);
	_whiteLumPercSlider->onValueChanged.Add(this, &HDRConfigurationPane::OnValueChanged);

	_bloomThresholdLabel = new Gwen::Controls::Label(this);
	_bloomThresholdLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_bloomThresholdLabel->SetBounds(0, _whiteLumPercSlider->Bottom() + spacing, childWidth, labelHight);

	_bloomThresholdSlider = new Gwen::Controls::HorizontalSlider(this);
	_bloomThresholdSlider->SetClampToNotches(false);
	_bloomThresholdSlider->SetRange(0.0f, 1.0f);
	_bloomThresholdSlider->SetValue(pp->GetBloomThreshold());
	_bloomThresholdSlider->SetBounds(0, _bloomThresholdLabel->Bottom(), childWidth, sliderHeight);
	_bloomThresholdSlider->onValueChanged.Add(this, &HDRConfigurationPane::OnValueChanged);

	_bloomMagnitudeLabel = new Gwen::Controls::Label(this);
	_bloomMagnitudeLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_bloomMagnitudeLabel->SetBounds(0, _bloomThresholdSlider->Bottom() + spacing, childWidth, labelHight);

	_bloomMagnitudeSlider = new Gwen::Controls::HorizontalSlider(this);
	_bloomMagnitudeSlider->SetClampToNotches(false);
	_bloomMagnitudeSlider->SetRange(0.0f, 5.0f);
	_bloomMagnitudeSlider->SetValue(pp->GetBloomMagnitude());
	_bloomMagnitudeSlider->SetBounds(0, _bloomMagnitudeLabel->Bottom(), childWidth, sliderHeight);
	_bloomMagnitudeSlider->onValueChanged.Add(this, &HDRConfigurationPane::OnValueChanged);

	_bloomBlurSigmaLabel = new Gwen::Controls::Label(this);
	_bloomBlurSigmaLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_bloomBlurSigmaLabel->SetBounds(0, _bloomMagnitudeSlider->Bottom() + spacing, childWidth, labelHight);

	_bloomBlurSigmaSlider = new Gwen::Controls::HorizontalSlider(this);
	_bloomBlurSigmaSlider->SetClampToNotches(false);
	_bloomBlurSigmaSlider->SetRange(0.0f, 4.0f);
	_bloomBlurSigmaSlider->SetValue(pp->GetBloomBlurSigma());
	_bloomBlurSigmaSlider->SetBounds(0, _bloomBlurSigmaLabel->Bottom(), childWidth, sliderHeight);
	_bloomBlurSigmaSlider->onValueChanged.Add(this, &HDRConfigurationPane::OnValueChanged);

	_exposureKeyLabel = new Gwen::Controls::Label(this);
	_exposureKeyLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_exposureKeyLabel->SetBounds(0, _bloomBlurSigmaSlider->Bottom() + spacing, childWidth, labelHight);

	_exposureKeySlider = new Gwen::Controls::HorizontalSlider(this);
	_exposureKeySlider->SetClampToNotches(false);
	_exposureKeySlider->SetRange(0.0f, 1.0f);
	_exposureKeySlider->SetValue(pp->GetExposureKey());
	_exposureKeySlider->SetBounds(0, _exposureKeyLabel->Bottom(), childWidth, sliderHeight);
	_exposureKeySlider->onValueChanged.Add(this, &HDRConfigurationPane::OnValueChanged);

	_timeScaleLabel = new Gwen::Controls::Label(this);
	_timeScaleLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_timeScaleLabel->SetBounds(0, _exposureKeySlider->Bottom() + spacing, childWidth, labelHight);

	_timeScaleSlider = new Gwen::Controls::HorizontalSlider(this);
	_timeScaleSlider->SetClampToNotches(false);
	_timeScaleSlider->SetRange(0.0f, 10.0f);
	_timeScaleSlider->SetValue(_timeScale);
	_timeScaleSlider->SetBounds(0, _timeScaleLabel->Bottom(), childWidth, sliderHeight);
	_timeScaleSlider->onValueChanged.Add(this, &HDRConfigurationPane::OnValueChanged);
}

HDRConfigurationPane::~HDRConfigurationPane()
{
}

void HDRConfigurationPane::OnValueChanged(Gwen::Controls::Base *control)
{
	HDRPostProcess* pp = GetConfiguredObject();

	if (control == _tauSlider)
	{
		pp->SetTau(_tauSlider->GetValue());
	}
	else if (control == _whiteLumPercSlider)
	{
		pp->SetWhiteLuminancePercentage(_whiteLumPercSlider->GetValue());
	}
	else if (control == _bloomThresholdSlider)
	{
		pp->SetBloomThreshold(_bloomThresholdSlider->GetValue());
	}
	else if (control == _bloomMagnitudeSlider)
	{
		pp->SetBloomMagnitude(_bloomMagnitudeSlider->GetValue());
	}
	else if (control == _bloomBlurSigmaSlider)
	{
		pp->SetBloomBlurSigma(_bloomBlurSigmaSlider->GetValue());
	}
	else if (control == _exposureKeySlider)
	{
		pp->SetExposureKey(_exposureKeySlider->GetValue());
	}
	else if (control == _timeScaleSlider)
	{
		_timeScale = _timeScaleSlider->GetValue();
	}
}

void HDRConfigurationPane::OnFrameMove(double totalTime, float dt)
{
	HDRPostProcess* pp = GetConfiguredObject();
	
	_tauSlider->SetValue(pp->GetTau());
	_tauLabel->SetText("Tau: " + Gwen::Utility::ToString(pp->GetTau()));

	_whiteLumPercSlider->SetValue(pp->GetWhiteLuminancePercentage());
	_whiteLumPercLabel->SetText("White luminance percentage: " + Gwen::Utility::ToString(pp->GetWhiteLuminancePercentage()));

	_bloomThresholdSlider->SetValue(pp->GetBloomThreshold());
	_bloomThresholdLabel->SetText("Bloom threshold: " + Gwen::Utility::ToString(pp->GetBloomThreshold()));

	_bloomMagnitudeSlider->SetValue(pp->GetBloomMagnitude());
	_bloomMagnitudeLabel->SetText("Bloom magnitude: " + Gwen::Utility::ToString(pp->GetBloomMagnitude()));
	
	_bloomBlurSigmaSlider->SetValue(pp->GetBloomBlurSigma());
	_bloomBlurSigmaLabel->SetText("Bloom blur sigma: " + Gwen::Utility::ToString(pp->GetBloomBlurSigma()));

	_exposureKeySlider->SetValue(pp->GetExposureKey());
	_exposureKeyLabel->SetText("Exposure key: " + Gwen::Utility::ToString(pp->GetExposureKey()));

	_timeScaleSlider->SetValue(_timeScale);
	pp->SetTimeDelta(dt * _timeScale);
	_timeScaleLabel->SetText("Time scale: " + Gwen::Utility::ToString(_timeScale));
}