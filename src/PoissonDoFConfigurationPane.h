#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "PoissonDoFPostProcess.h"
#include "SliderWithLabel.h"

class PoissonDoFConfigurationPane : public ConfigurationPane<PoissonDoFPostProcess>
{
private:
	SliderWithLabel* _focalDistSlider;
	SliderWithLabel* _falloffStartSlider;
	SliderWithLabel* _falloffEndSlider;
	SliderWithLabel* _cocScaleSlider;
	SliderWithLabel* _sampleCountSlider;

	virtual void OnValueChanged(Gwen::Controls::Base *control);

public:
	PoissonDoFConfigurationPane(Gwen::Controls::Base* parent, PoissonDoFPostProcess* pp);

	void OnFrameMove(double totalTime, float dt);
};