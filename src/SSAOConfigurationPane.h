#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "SSAOPostProcess.h"
#include "SliderWithLabel.h"

class SSAOConfigurationPane : public ConfigurationPane<SSAOPostProcess>
{
private:
	SliderWithLabel* _sampleRadiusSlider;
	SliderWithLabel* _blurSigmaSlider;
	SliderWithLabel* _samplePowerSlider;
	SliderWithLabel* _sampleCountSlider;

	virtual void OnValueChanged(Gwen::Controls::Base *control);

public:
	SSAOConfigurationPane(Gwen::Controls::Base* parent, SSAOPostProcess* pp);

	void OnFrameMove(double totalTime, float dt);
};