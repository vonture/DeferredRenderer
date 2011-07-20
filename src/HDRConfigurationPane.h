#pragma once

#include "Defines.h"
#include "ConfigurationPane.h"
#include "HDRPostProcess.h"
#include "Gwen/Controls/HorizontalSlider.h"

class HDRConfigurationPane : public ConfigurationPane<HDRPostProcess>
{
private:
	Gwen::Controls::HorizontalSlider* _tauSlider;
	Gwen::Controls::HorizontalSlider* _whiteLumPercSlider;
	Gwen::Controls::HorizontalSlider* _bloomThresholdSlider;
	Gwen::Controls::HorizontalSlider* _bloomMagnitudeSlider;
	Gwen::Controls::HorizontalSlider* _bloomBlurSigmaSlider;
	Gwen::Controls::HorizontalSlider* _exposureKeySlider;

	Gwen::Controls::Label* _tauLabel;
	Gwen::Controls::Label* _whiteLumPercLabel;
	Gwen::Controls::Label* _bloomThresholdLabel;
	Gwen::Controls::Label* _bloomMagnitudeLabel;
	Gwen::Controls::Label* _bloomBlurSigmaLabel;
	Gwen::Controls::Label* _exposureKeyLabel;

public:
	HDRConfigurationPane(Gwen::Controls::Base* parent, HDRPostProcess* pp);
	~HDRConfigurationPane();

	void OnFrameMove(double totalTime, float dt);
};