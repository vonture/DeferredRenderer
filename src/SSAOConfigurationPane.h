#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "SSAOPostProcess.h"
#include "Gwen/Controls/HorizontalSlider.h"

class SSAOConfigurationPane : public ConfigurationPane<SSAOPostProcess>
{
private:
	Gwen::Controls::HorizontalSlider* _sampleRadiusSlider;
	Gwen::Controls::HorizontalSlider* _blurSigmaSlider;
	Gwen::Controls::HorizontalSlider* _samplePowerSlider;
	Gwen::Controls::HorizontalSlider* _sampleCountSlider;

	Gwen::Controls::Label* _sampleRadiusLabel;
	Gwen::Controls::Label* _blurSigmaLabel;
	Gwen::Controls::Label* _samplePowerLabel;
	Gwen::Controls::Label* _sampleCountLabel;

	virtual void OnValueChanged(Gwen::Controls::Base *control);

public:
	SSAOConfigurationPane(Gwen::Controls::Base* parent, SSAOPostProcess* pp);

	void OnFrameMove(double totalTime, float dt);
};