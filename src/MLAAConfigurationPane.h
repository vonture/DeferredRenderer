#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "MLAAPostProcess.h"
#include "SliderWithLabel.h"
#include "Gwen/Controls/CheckBox.h"

class MLAAConfigurationPane : public ConfigurationPane<MLAAPostProcess>
{
private:
	Gwen::Controls::CheckBoxWithLabel* _depthDetectionCheckBox;
	SliderWithLabel* _depthThresholdSlider;

	Gwen::Controls::CheckBoxWithLabel* _normalDetectionCheckBox;
	SliderWithLabel* _normalThresholdSlider;
	
	Gwen::Controls::CheckBoxWithLabel* _lumDetectionCheckBox;
	SliderWithLabel* _lumThresholdSlider;

	SliderWithLabel* _maxSearchStepsSlider;

	virtual void OnValueChanged(Gwen::Controls::Base *control);

public:
	MLAAConfigurationPane(Gwen::Controls::Base* parent, MLAAPostProcess* pp);

	void OnFrameMove(double totalTime, float dt);
};