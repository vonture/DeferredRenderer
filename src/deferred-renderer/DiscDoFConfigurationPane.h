#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "DiscDoFPostProcess.h"
#include "SliderWithLabel.h"

class DiscDoFConfigurationPane : public ConfigurationPane<DiscDoFPostProcess>
{
private:
	SliderWithLabel* _focalDistSlider;
	SliderWithLabel* _falloffStartSlider;
	SliderWithLabel* _falloffEndSlider;
	SliderWithLabel* _cocScaleSlider;
	SliderWithLabel* _sampleCountSlider;

	virtual void OnValueChanged(Gwen::Controls::Base *control);

public:
	DiscDoFConfigurationPane(Gwen::Controls::Base* parent, DiscDoFPostProcess* pp);

	void OnFrameMove(double totalTime, float dt);
};