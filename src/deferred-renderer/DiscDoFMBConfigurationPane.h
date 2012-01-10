#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "DiscDoFMBPostProcess.h"
#include "SliderWithLabel.h"

class DiscDoFMBConfigurationPane : public ConfigurationPane<DiscDoFMBPostProcess>
{
private:
	SliderWithLabel* _focalDistSlider;
	SliderWithLabel* _falloffStartSlider;
	SliderWithLabel* _falloffEndSlider;
	SliderWithLabel* _cocScaleSlider;
	SliderWithLabel* _sampleCountSlider;

	virtual void OnValueChanged(Gwen::Controls::Base *control);

public:
	DiscDoFMBConfigurationPane(Gwen::Controls::Base* parent, DiscDoFMBPostProcess* pp);

	void OnFrameMove(double totalTime, float dt);
};